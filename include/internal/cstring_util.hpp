
/*
 * Created: 31-08-2018
 * Copyright (c) <2018> <Jonas R. Glesaaen (jonas@glesaaen.com)>
 */

#ifndef GTEST_MPI_LISTENER_CSTRING_UTIL_HPP
#define GTEST_MPI_LISTENER_CSTRING_UTIL_HPP

#include <cstring>

namespace gtest_mpi_listener {
namespace internal {

// Compares two C strings  Returns true iff they have the same content.
// Source: googletest/src/gtest.cc
inline bool CStringEquals(const char *lhs, const char *rhs)
{
  if (lhs == nullptr) {
    return rhs == nullptr;
  }

  if (rhs == nullptr) {
    return false;
  }

  return strcmp(lhs, rhs) == 0;
}

// Compares two C strings, ignoring case.  Returns true iff they have the same
// content.
// Source: googletest/src/gtest.cc
inline bool CaseInsensitiveCStringEquals(const char *lhs, const char *rhs)
{
  if (lhs == nullptr) {
    return rhs == nullptr;
  }

  if (rhs == nullptr) {
    return false;
  }

  return strcasecmp(lhs, rhs) == 0;
}

} // namespace internal
} // namespace gtest_mpi_listener

#endif /* GTEST_MPI_LISTENER_CSTRING_UTIL_HPP */
