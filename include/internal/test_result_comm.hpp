
/*
 * Created: 31-08-2018
 * Copyright (c) <2018> <Jonas R. Glesaaen (jonas@glesaaen.com)>
 */

#ifndef GTEST_MPI_LISTENER_TEST_RESULT_COMM_HPP
#define GTEST_MPI_LISTENER_TEST_RESULT_COMM_HPP

#include "gtest/gtest.h"
#include <mpi.h>
#include <vector>

namespace gtest_mpi_listener {
namespace internal {

struct Test_Result
{
  ::testing::TestPartResult::Type type;
  int line_number;
  std::string filename;
  std::string message;
};

class Test_Result_Communicator
{
public:
  using test_type = ::testing::TestPartResult;
  using test_container_type = std::vector<test_type>;
  using test_iterator_type = test_container_type::const_iterator;

  class Test_Iterator;
  class Send_Iterator;
  class Recieve_Iterator;

  Test_Result_Communicator(test_container_type const &results, MPI_Comm comm)
      : test_results_(results)
  {
    MPI_Comm_dup(comm, &comm_);

    auto flag = MPI_Comm_rank(comm_, &rank_);

    if (flag != MPI_SUCCESS) {
      throw std::runtime_error{
          "Unable to get MPI rank in Test_Result_Communicator constructor"};
    }

    flag = MPI_Comm_size(comm_, &size_);

    if (flag != MPI_SUCCESS) {
      throw std::runtime_error{
          "Unable to get MPI size in Test_Result_Communicator constructor"};
    }

    partial_count_.resize(size_);

    int local_partial_count = test_results_.size();
    MPI_Gather(&local_partial_count, 1, MPI_INT, partial_count_.data(), 1,
               MPI_INT, 0, comm_);
  }

  Test_Iterator test_iterator();
  Send_Iterator send_iterator();
  Recieve_Iterator recieve_iterator(int source);

private:
  int rank_;
  int size_;
  MPI_Comm comm_;

  test_container_type const &test_results_;
  std::vector<int> partial_count_;
};

class Test_Result_Communicator::Test_Iterator
{
public:
  Test_Iterator(Test_Result_Communicator const &owner)
      : it_{owner.test_results_.begin()}, owner_(owner){};

  Test_Result const operator*()
  {
    return {it_->type(), it_->line_number(), std::string{it_->file_name()},
            std::string{it_->message()}};
  }

  Test_Iterator &operator++()
  {
    ++it_;
    return *this;
  }

  bool end() const
  {
    return it_ == owner_.test_results_.end();
  }

protected:
  test_iterator_type it_;
  Test_Result_Communicator const &owner_;
};

class Test_Result_Communicator::Send_Iterator
    : public Test_Result_Communicator::Test_Iterator
{
public:
  Send_Iterator(Test_Result_Communicator const &owner) : Test_Iterator(owner)
  {
    if (owner_.rank_ == 0) {
      throw std::runtime_error{"Send_Iterator cannot be constructed on rank 0"};
    }
  }

  void send()
  {
    auto result_type = static_cast<int>(it_->type());
    auto result_filename = std::string{it_->file_name()};
    auto result_line_number = it_->line_number();
    auto result_message = std::string{it_->message()};

    auto filename_length = static_cast<int>(result_filename.size());
    auto message_length = static_cast<int>(result_message.size());

    auto my_rank = owner_.rank_;
    auto comm = owner_.comm_;

    MPI_Send(&result_type, 1, MPI_INT, 0, my_rank, comm);
    MPI_Send(&filename_length, 1, MPI_INT, 0, my_rank, comm);
    MPI_Send(&result_line_number, 1, MPI_INT, 0, my_rank, comm);
    MPI_Send(&message_length, 1, MPI_INT, 0, my_rank, comm);
    MPI_Send(const_cast<char *>(result_filename.c_str()), filename_length,
             MPI_CHAR, 0, my_rank, comm);
    MPI_Send(const_cast<char *>(result_message.c_str()), message_length,
             MPI_CHAR, 0, my_rank, comm);
  }
};

class Test_Result_Communicator::Recieve_Iterator
{
public:
  Recieve_Iterator(Test_Result_Communicator const &owner, int source)
      : owner_(owner), pos_{0}, source_rank_{source}
  {
    if (owner_.rank_ != 0) {
      throw std::runtime_error{
          "Revieve_Iterator cannot be constructed on a rank other than 0"};
    }

    if (owner_.rank_ > owner_.size_) {
      throw std::runtime_error{"Revieve_Iterator rank out of bounds"};
    }
  }

  Test_Result recieve()
  {
    Test_Result result;
    MPI_Comm comm = owner_.comm_;

    MPI_Recv(&result.type, 1, MPI_INT, source_rank_, source_rank_, comm,
             MPI_STATUS_IGNORE);

    int filename_length;
    MPI_Recv(&filename_length, 1, MPI_INT, source_rank_, source_rank_, comm,
             MPI_STATUS_IGNORE);
    MPI_Recv(&result.line_number, 1, MPI_INT, source_rank_, source_rank_, comm,
             MPI_STATUS_IGNORE);

    int message_length;
    MPI_Recv(&message_length, 1, MPI_INT, source_rank_, source_rank_, comm,
             MPI_STATUS_IGNORE);

    std::vector<char> filename_str(filename_length);
    MPI_Recv(filename_str.data(), filename_length, MPI_CHAR, source_rank_,
             source_rank_, comm, MPI_STATUS_IGNORE);
    result.filename = std::string{filename_str.begin(), filename_str.end()};

    std::vector<char> message_str(message_length);
    MPI_Recv(message_str.data(), message_length, MPI_CHAR, source_rank_,
             source_rank_, comm, MPI_STATUS_IGNORE);
    result.message = std::string{message_str.begin(), message_str.end()};

    return result;
  }

  Recieve_Iterator &operator++()
  {
    ++pos_;
    return *this;
  }

  bool end() const
  {
    return pos_ == owner_.partial_count_[source_rank_];
  }

private:
  Test_Result_Communicator const &owner_;
  int pos_;
  int source_rank_;
};

inline Test_Result_Communicator::Test_Iterator
Test_Result_Communicator::test_iterator()
{
  return Test_Iterator{*this};
}

Test_Result_Communicator::Send_Iterator
Test_Result_Communicator::send_iterator()
{
  return Send_Iterator{*this};
}

Test_Result_Communicator::Recieve_Iterator
Test_Result_Communicator::recieve_iterator(int source)
{
  return Recieve_Iterator{*this, source};
}

} // namespace internal
} // namespace gtest_mpi_listener

#endif /* GTEST_MPI_LISTENER_TEST_RESULT_COMM_HPP */
