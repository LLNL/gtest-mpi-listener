
/*
 * Created: 31-08-2018
 * Copyright (c) <2018> <Jonas R. Glesaaen (jonas@glesaaen.com)>
 */

#ifndef GTEST_MPI_LISTENER_PRETTY_PRINT_HPP
#define GTEST_MPI_LISTENER_PRETTY_PRINT_HPP

#include "cstring_util.hpp"
#include "gtest/gtest.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <sstream>
#include <unistd.h>

namespace gtest_mpi_listener {
namespace internal {

enum GTestColor
{
  COLOR_DEFAULT,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW
};

namespace {

// Returns true iff Google Test should use colors in the output.
// Source: googletest/src/gtest.cc
inline bool ShouldUseColor(bool stdout_is_tty)
{
  using namespace ::testing;
  const char *const gtest_color = GTEST_FLAG(color).c_str();

  if (CaseInsensitiveCStringEquals(gtest_color, "auto")) {
    const char *const term = std::getenv("TERM");

    const bool term_supports_color =
        CStringEquals(term, "xterm") || CStringEquals(term, "xterm-color") ||
        CStringEquals(term, "xterm-256color") ||
        CStringEquals(term, "screen") ||
        CStringEquals(term, "screen-256color") || CStringEquals(term, "tmux") ||
        CStringEquals(term, "tmux-256color") ||
        CStringEquals(term, "rxvt-unicode") ||
        CStringEquals(term, "rxvt-unicode-256color") ||
        CStringEquals(term, "linux") || CStringEquals(term, "cygwin");

    return stdout_is_tty && term_supports_color;
  }

  return CaseInsensitiveCStringEquals(gtest_color, "yes") ||
         CaseInsensitiveCStringEquals(gtest_color, "true") ||
         CaseInsensitiveCStringEquals(gtest_color, "t") ||
         CStringEquals(gtest_color, "1");
}

// Returns the ANSI color code for the given color.  COLOR_DEFAULT is
// an invalid input.
// Source: googletest/src/gtest.cc
inline const char *GetAnsiColorCode(GTestColor color)
{
  switch (color) {
  case COLOR_RED:
    return "1";
  case COLOR_GREEN:
    return "2";
  case COLOR_YELLOW:
    return "3";
  default:
    return NULL;
  };
}

} // namespace

inline void ColoredPrintf(GTestColor color, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  static const bool in_color_mode = ShouldUseColor(isatty(fileno(stdout)) != 0);

  const bool use_color = in_color_mode && (color != COLOR_DEFAULT);

  if (use_color) {
    printf("\033[0;3%sm", GetAnsiColorCode(color));
  }

  vprintf(fmt, args);

  if (use_color) {
    printf("\033[m"); // Resets the terminal to default.
  }

  va_end(args);
}

// Formats a countable noun.  Depending on its quantity, either the
// singular form or the plural form is used.
// Source: googletest/src/gtest.cc
inline std::string FormatCountableNoun(int count, const char *singular_form,
                                       const char *plural_form)
{
  return ::testing::internal::StreamableToString(count) + " " +
         (count == 1 ? singular_form : plural_form);
}

// Formats the count of tests.
// Source: googletest/src/gtest.cc
inline std::string FormatTestCount(int test_count)
{
  return FormatCountableNoun(test_count, "test", "tests");
}

// Formats the count of test cases.
// Source: googletest/src/gtest.cc
inline std::string FormatTestCaseCount(int test_case_count)
{
  return FormatCountableNoun(test_case_count, "test case", "test cases");
}

// Formates the count of MPI processes.
inline std::string FormatMPIRankCount(int mpi_size)
{
  return FormatCountableNoun(mpi_size, "MPI process", "MPI processes");
}

// Converts a TestPartResult::Type enum to human-friendly string
// representation.  Both kNonFatalFailure and kFatalFailure are translated
// to "Failure", as the user usually doesn't care about the difference
// between the two when viewing the test result.
// Source: googletest/src/gtest.cc
inline const char *
TestPartResultTypeToString(::testing::TestPartResult::Type type)
{
  switch (type) {
  case ::testing::TestPartResult::kSuccess:
    return "Success";
  case ::testing::TestPartResult::kNonFatalFailure:
  case ::testing::TestPartResult::kFatalFailure:
    return "Failure";
  default:
    return "Unknown result type";
  }
}

// Converts a set of integer to appear on ranged form. So e.g. a set containing
// the numbers {0, 1, 2, 5, 7, 8, 10} will be "0-2,5,7-8,10"
inline std::string FormatRangedSet(std::set<int> const &s)
{
  std::stringstream ss;
  auto it = s.begin();

  while (it != s.end()) {
    auto stop = it;
    ++stop;

    // Look for the first non-consecutive entry
    for (int i = 1; stop != s.end(); ++i, ++stop) {
      if (*stop != *it + i) {
        --stop;
        break;
      }
    }

    if (stop == s.end()) {
      --stop;
    }

    // If the distance between them are more than 1, it is a range, if not it is
    // a single entry
    if (std::distance(it, stop) == 0ul) {
      ss << *it;
    } else {
      ss << *it << "-" << *stop;
    }

    // Only add a comma if there are more entries
    if (std::distance(stop, s.end()) > 1u) {
      ss << ",";
    }

    it = ++stop;
  }

  return ss.str();
}

} // namespace internal
} // namespace gtest_mpi_listener

#endif /* GTEST_MPI_LISTENER_PRETTY_PRINT_HPP */
