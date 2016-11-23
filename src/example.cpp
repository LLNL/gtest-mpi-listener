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
