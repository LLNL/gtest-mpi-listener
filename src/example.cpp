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

#include "gtest/gtest.h"
#include "gtest-mpi-listener.hpp"
#include "mpi.h"

// Simple-minded functions for some testing

// Always passes out == rank
int getMpiRank(MPI_Comm comm) {
  int out;
  MPI_Comm_rank(comm, &out);
  return out;
}

// Always fails out == rank
int getMpiRankPlusOne(MPI_Comm comm) {
  int out;
  MPI_Comm_rank(comm, &out);
  return (out+1);
}

// Passes out == rank when rank is zero, fails otherwise
int getZero(MPI_Comm comm) {
  return 0;
}

// Passes out == rank except on rank zero, fails otherwise
int getNonzeroMpiRank(MPI_Comm comm) {
  int out;
  MPI_Comm_rank(comm, &out);
  return (out ? out : 1);
}

// These tests could be made shorter with a fixture, but a fixture
// deliberately isn't used in order to make the test harness extremely simple
TEST(BasicMPI, PassOnAllRanks) {
  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Comm_rank(comm, &rank);
  EXPECT_EQ(rank, getMpiRank(comm));
}

TEST(BasicMPI, FailOnAllRanks) {
  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Comm_rank(comm, &rank);
  EXPECT_EQ(rank, getMpiRankPlusOne(comm));
}

TEST(BasicMPI, FailExceptOnRankZero) {
  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Comm_rank(comm, &rank);
  EXPECT_EQ(rank, getZero(comm));
}

TEST(BasicMPI, PassExceptOnRankZero) {
  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Comm_rank(comm, &rank);
  EXPECT_EQ(rank, getNonzeroMpiRank(comm));
}

int main(int argc, char** argv) {
  // Filter out Google Test arguments
  ::testing::InitGoogleTest(&argc, argv);

  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Add object that will finalize MPI on exit; Google Test owns this pointer
  ::testing::AddGlobalTestEnvironment(new MPIEnvironment);

  // Get the event listener list.
  ::testing::TestEventListeners& listeners =
      ::testing::UnitTest::GetInstance()->listeners();

  // Remove default listener
  delete listeners.Release(listeners.default_result_printer());

  // Adds MPI listener; Google Test owns this pointer
  listeners.Append(new MPIMinimalistPrinter);

  // Run tests, then clean up and exit
  RUN_ALL_TESTS();

  return 0;
}
