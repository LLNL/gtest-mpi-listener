/*******************************************************************************
 *
 * Copyright (c) 2016-2018, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory
 * Written by Geoffrey M. Oxberry <oxberry1@llnl.gov>
 * LLNL-CODE-739313
 * All rights reserved.
 *
 * This file is part of gtest-mpi-listener. For details, see
 * https://github.com/LLNL/gtest-mpi-listener
 *
 * Please also see the LICENSE and COPYRIGHT files for the BSD-3
 * license and copyright information.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the disclaimer below.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the disclaimer (as noted below)
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 * * Neither the name of the LLNS/LLNL nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
 * LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************/
//
/*******************************************************************************
 * An example from Google Test was copied with minor modifications. The
 * license of Google Test is below.
 *
 * Google Test has the following copyright notice, which must be
 * duplicated in its entirety per the terms of its license:
 *
 *  Copyright 2005, Google Inc.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *      * Neither the name of Google Inc. nor the names of its
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef GTEST_MPI_MINIMAL_LISTENER_H
#define GTEST_MPI_MINIMAL_LISTENER_H

#include "internal/cstring_util.hpp"
#include "internal/pretty_print.hpp"
#include "internal/test_result_comm.hpp"
#include "mpi.h"
#include "gtest/gtest.h"
#include <cassert>
#include <vector>

namespace gtest_mpi_listener {

// This class sets up the global test environment, which is needed
// to finalize MPI.
class MPIEnvironment : public ::testing::Environment
{
public:
  MPIEnvironment() : ::testing::Environment() {}

  virtual ~MPIEnvironment() {}

  virtual void SetUp()
  {
    int is_mpi_initialized;
    ASSERT_EQ(MPI_Initialized(&is_mpi_initialized), MPI_SUCCESS);

    if (!is_mpi_initialized) {
      printf("MPI must be initialized before RUN_ALL_TESTS!\n");
      printf("Add '::testing::InitGoogleTest(&argc, argv);\n");
      printf("     MPI_Init(&argc, &argv);' to your 'main' function!\n");
      FAIL();
    }
  }

  virtual void TearDown()
  {
    int is_mpi_finalized;
    ASSERT_EQ(MPI_Finalized(&is_mpi_finalized), MPI_SUCCESS);

    if (!is_mpi_finalized) {
      int rank;
      ASSERT_EQ(MPI_Comm_rank(MPI_COMM_WORLD, &rank), MPI_SUCCESS);
      ASSERT_EQ(MPI_Finalize(), MPI_SUCCESS);
    }

    ASSERT_EQ(MPI_Finalized(&is_mpi_finalized), MPI_SUCCESS);
    ASSERT_TRUE(is_mpi_finalized);
  }

private:
  // Disallow copying
  MPIEnvironment(const MPIEnvironment &) {}
};

// An empty event listener that stores the current MPI state
class EmptyMPIEventListener : public ::testing::EmptyTestEventListener
{
public:
  EmptyMPIEventListener() : EmptyMPIEventListener(MPI_COMM_WORLD){};

  EmptyMPIEventListener(MPI_Comm comm)
  {
    int is_mpi_initialized = 0;
    assert(MPI_Initialized(&is_mpi_initialized) == MPI_SUCCESS);

    if (!is_mpi_initialized) {
      printf("MPI must be initialized before RUN_ALL_TESTS!\n");
      printf("Add '::testing::InitGoogleTest(&argc, argv);\n");
      printf("     MPI_Init(&argc, &argv);' to your 'main' function!\n");
      assert(0);
    }

    MPI_Comm_dup(comm, &comm_);
    UpdateCommState();
  }

  EmptyMPIEventListener(const EmptyMPIEventListener &printer)
      : EmptyMPIEventListener(printer.comm_){};

  virtual ~EmptyMPIEventListener() {}

protected:
  MPI_Comm comm_;
  int rank_;
  int size_;

private:
  int UpdateCommState()
  {
    int flag = MPI_Comm_rank(comm_, &rank_);

    if (flag != MPI_SUCCESS) {
      return flag;
    }

    flag = MPI_Comm_size(comm_, &size_);
    return flag;
  }
};

// This class more or less takes the code in Google Test's
// MinimalistPrinter example and wraps certain parts of it in MPI calls,
// gathering all results onto rank zero.
class MPIMinimalistPrinter : public EmptyMPIEventListener
{
public:
  using EmptyMPIEventListener::EmptyMPIEventListener;

  MPIMinimalistPrinter(const MPIMinimalistPrinter &printer)
      : EmptyMPIEventListener(printer), result_vector(printer.result_vector){};

  virtual ~MPIMinimalistPrinter(){};

  // Called before a test starts.
  void OnTestStart(const ::testing::TestInfo &test_info) override
  {
    // Only need to report test start info on rank 0
    if (rank_ == 0) {
      printf("*** Test %s.%s starting.\n", test_info.test_case_name(),
             test_info.name());
    }
  }

  // Called after an assertion failure or an explicit SUCCESS() macro.
  // In an MPI program, this means that certain ranks may not call this
  // function if a test part does not fail on all ranks. Consequently, it
  // is difficult to have explicit synchronization points here.
  void
  OnTestPartResult(const ::testing::TestPartResult &test_part_result) override
  {
    result_vector.push_back(test_part_result);
  }

  // Called after a test ends.
  void OnTestEnd(const ::testing::TestInfo &test_info) override
  {
    using namespace internal;
    std::set<int> failed_ranks;
    Test_Result_Communicator result_comm{result_vector, comm_};

    if (rank_ != 0) {
      // Ranks other than 0 sends its data to rank 0
      for (auto send_it = result_comm.send_iterator(); !send_it.end();
           ++send_it) {
        send_it.send();
      }
    } else {
      // Rank 0 first prints its local result data
      for (auto res_it = result_comm.test_iterator(); !res_it.end(); ++res_it) {
        auto result = *res_it;

        PrintTestResult(result, 0);

        if (result.type != ::testing::TestPartResult::Type::kSuccess) {
          failed_ranks.insert(0);
        }
      }

      // Rank 0 then fetches result data from the rest of the MPI processes
      for (int r = 1; r < size_; ++r) {
        for (auto rescv_it = result_comm.recieve_iterator(r); !rescv_it.end();
             ++rescv_it) {
          auto result = rescv_it.recieve();

          PrintTestResult(result, r);

          if (result.type != ::testing::TestPartResult::Type::kSuccess) {
            failed_ranks.insert(r);
          }
        }
      }
    }

    PrintTestEnd(test_info, failed_ranks);
    AppendFailedTest(test_info, failed_ranks);
    result_vector.clear();
  }

  virtual void PrintTestResult(internal::Test_Result const &result,
                               int res_rank)
  {
    if (rank_ != 0) {
      return;
    }

    printf("      %s on rank %d, %s:%d\n%s\n",
           (result.type != ::testing::TestPartResult::Type::kSuccess)
               ? "*** Failure"
               : "Success",
           res_rank, result.filename.c_str(), result.line_number,
           result.message.c_str());
  }

  virtual void PrintTestEnd(const ::testing::TestInfo &test_info,
                            std::set<int> const &) const
  {
    if (rank_ != 0) {
      return;
    }

    printf("*** Test %s.%s ending.\n", test_info.test_case_name(),
           test_info.name());
  }

  virtual void AppendFailedTest(const ::testing::TestInfo &,
                                std::set<int> const &){};

private:
  std::vector<::testing::TestPartResult> result_vector;
};

// A variant of the MPIMinimalistPrinter that tries to be as true to the default
// output googletest gives as possible, but still including additional
// information from MPI such as which rank the failure was on.
class PrettyMPIPrinter : public MPIMinimalistPrinter
{
public:
  using MPIMinimalistPrinter::MPIMinimalistPrinter;

  virtual ~PrettyMPIPrinter(){};

  // Overloaded functions for the TestEventListener

  // Fired before each iteration of tests starts.
  // Source: googletest/src/gtest.cc
  void OnTestIterationStart(const ::testing::UnitTest &unit_test,
                            int iteration) override
  {
    using namespace ::testing;
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    if (GTEST_FLAG(repeat) != 1) {
      printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);
    }

    const char *const filter = GTEST_FLAG(filter).c_str();

    // Prints the filter if it's not *. This reminds the user that some
    // tests may be skipped.
    if (!CStringEquals(filter, "*")) {
      ColoredPrintf(COLOR_YELLOW, "Note: %s filter = %s\n", GTEST_NAME_,
                    filter);
    }

    if (GTEST_FLAG(shuffle)) {
      ColoredPrintf(COLOR_YELLOW,
                    "Note: Randomizing tests' orders with a seed of %d .\n",
                    unit_test.random_seed());
    }

    ColoredPrintf(COLOR_GREEN, "[==========] ");

    printf("Running %s from %s using %s.\n",
           FormatTestCount(unit_test.test_to_run_count()).c_str(),
           FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str(),
           FormatMPIRankCount(size_).c_str());

    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnEnvironmentsSetUpStart(const ::testing::UnitTest &) override
  {
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    ColoredPrintf(COLOR_GREEN, "[----------] ");
    printf("Global MPI test environment set-up.\n");
    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnTestCaseStart(const ::testing::TestCase &test_case) override
  {
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    const std::string counts =
        FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");

    ColoredPrintf(COLOR_GREEN, "[----------] ");

    printf("%s from %s", counts.c_str(), test_case.name());

    if (test_case.type_param() == NULL) {
      printf("\n");
    } else {
      printf(", where TypeParam = %s\n", test_case.type_param());
    }

    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnTestStart(const ::testing::TestInfo &test_info) override
  {
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    ColoredPrintf(COLOR_GREEN, "[ RUN      ] ");
    printf("%s.%s\n", test_info.test_case_name(), test_info.name());
    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnTestCaseEnd(const ::testing::TestCase &test_case) override
  {
    using namespace internal;
    using namespace ::testing;

    if (!GTEST_FLAG(print_time))
      return;

    // Collect the runtimes of each thread, test runtime will be the max time
    auto time_in_ms = test_case.elapsed_time();
    auto full_time_in_ms = 0;
    MPI_Reduce(&time_in_ms, &full_time_in_ms, 1, MPI_LONG_LONG_INT, MPI_MAX, 0,
               comm_);

    if (rank_ != 0) {
      return;
    }

    const std::string counts =
        FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");

    ColoredPrintf(COLOR_GREEN, "[----------] ");

    printf("%s from %s (%s ms total)\n\n", counts.c_str(), test_case.name(),
           ::testing::internal::StreamableToString(full_time_in_ms).c_str());

    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnEnvironmentsTearDownStart(const ::testing::UnitTest &) override
  {
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    ColoredPrintf(COLOR_GREEN, "[----------] ");
    printf("Global MPI test environment tear-down\n");
    fflush(stdout);
  }

  // Source: googletest/src/gtest.cc
  void OnTestIterationEnd(const ::testing::UnitTest &unit_test, int) override
  {
    using namespace internal;
    using namespace ::testing;

    if (rank_ != 0) {
      return;
    }

    ColoredPrintf(COLOR_GREEN, "[==========] ");

    printf("%s from %s ran.",
           FormatTestCount(unit_test.test_to_run_count()).c_str(),
           FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());

    if (GTEST_FLAG(print_time)) {
      printf(" (%s ms total)",
             ::testing::internal::StreamableToString(unit_test.elapsed_time())
                 .c_str());
    }
    printf("\n");

    auto num_failures = failed_tests.size();

    ColoredPrintf(COLOR_GREEN, "[  PASSED  ] ");
    printf(
        "%s.\n",
        FormatTestCount(unit_test.total_test_count() - num_failures).c_str());

    if (num_failures > 0) {
      ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
      printf("%s, listed below:\n", FormatTestCount(num_failures).c_str());
      PrintFailedTests();

      printf("\n%s\n",
             FormatCountableNoun(num_failures, "FAILED TEST", "FAILED TESTS")
                 .c_str());
    }

    int num_disabled = unit_test.reportable_disabled_test_count();
    if (num_disabled && !GTEST_FLAG(also_run_disabled_tests)) {
      if (!num_failures) {
        printf("\n"); // Add a spacer if no FAILURE banner is displayed.
      }
      ColoredPrintf(
          COLOR_YELLOW, "YOU HAVE %s\n\n",
          FormatCountableNoun(num_disabled, "DISABLED TEST", "DISABLED TESTS")
              .c_str());
    }

    fflush(stdout);
  }

  // Functions special for the MPI interface

  // Print the failure of a specific node
  void PrintTestResult(internal::Test_Result const &result,
                       int res_rank) override
  {
    using namespace ::testing;
    using namespace internal;

    if (rank_ != 0) {
      return;
    }

    if (result.type == TestPartResult::Type::kSuccess) {
      return;
    }

    auto summary_as_string =
        (Message() << ::testing::internal::FormatFileLocation(
                          result.filename.c_str(), result.line_number)
                   << " " << TestPartResultTypeToString(result.type)
                   << " on rank " << res_rank << "\n"
                   << result.message)
            .GetString();

    printf("%s\n", summary_as_string.c_str());
    fflush(stdout);
  }

  // Summary print at the end of a test, including timing
  void PrintTestEnd(const ::testing::TestInfo &test_info,
                    std::set<int> const &failed_ranks) const override
  {
    using namespace ::testing;
    using namespace internal;

    // Collect the runtimes of each thread, test runtime will be the max time
    auto time_in_ms = test_info.result()->elapsed_time();
    auto full_time_in_ms = 0;
    MPI_Reduce(&time_in_ms, &full_time_in_ms, 1, MPI_LONG_LONG_INT, MPI_MAX, 0,
               comm_);

    if (rank_ != 0) {
      return;
    }

    if (failed_ranks.empty()) {
      ColoredPrintf(COLOR_GREEN, "[       OK ] ");
    } else {
      ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
    }

    printf("%s.%s", test_info.test_case_name(), test_info.name());

    if (!failed_ranks.empty()) {
      printf(" on %s {", failed_ranks.size() == 1 ? "rank" : "ranks");
      ColoredPrintf(COLOR_RED, "%s", FormatRangedSet(failed_ranks).c_str());
      printf("}");
    }

    if (GTEST_FLAG(print_time)) {
      printf(" (%s ms)\n",
             ::testing::internal::StreamableToString(full_time_in_ms).c_str());
    } else {
      printf("\n");
    }

    fflush(stdout);
  }

  // Add test failures to the list of failures on rank 0
  void AppendFailedTest(const ::testing::TestInfo &test_info,
                        std::set<int> const &failed_ranks) override
  {
    if (rank_ != 0 or failed_ranks.empty()) {
      return;
    }

    failed_tests.push_back(
        Failed_Tests_Summary{std::string{test_info.test_case_name()},
                             std::string{test_info.name()}, failed_ranks});
  }

  // Print the list of failed tests
  virtual void PrintFailedTests() const
  {
    using namespace internal;

    for (auto failure : failed_tests) {
      ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
      printf("%s.%s on %s {", failure.case_name.c_str(),
             failure.test_name.c_str(),
             failure.failed_ranks.size() == 1 ? "rank" : "ranks");
      ColoredPrintf(COLOR_RED, "%s",
                    FormatRangedSet(failure.failed_ranks).c_str());
      printf("}\n");
    }
  }

private:
  struct Failed_Tests_Summary
  {
    std::string case_name;
    std::string test_name;
    std::set<int> failed_ranks;
  };

  std::vector<Failed_Tests_Summary> failed_tests;
};

} // namespace gtest_mpi_listener

#endif /* GTEST_MPI_MINIMAL_LISTENER_H */
