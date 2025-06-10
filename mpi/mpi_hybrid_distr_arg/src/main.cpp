// Copyright 2024 ECCI-UCR CC-BY-4
#include <iostream>

#include <omp.h>
#include <algorithm>
#include <cstdlib>

#include "Mpi.hpp"

int calculate_start(int rank, int end, int workers, int begin);
int calculate_finish(int rank, int end, int workers, int begin);

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    int overall_start = -1;
    int overall_finish = -1;
    // NOTE: a deadlock could occur with cin, thus it's better to
    // avoid interactivity in distributed programs. Use point to point
    // communication for this
    // If start and finish provided in args, use them
    if (argc == 3) {
      overall_start = atoi(argv[1]);
      overall_finish = atoi(argv[2]);
    } else {
      // Process 0 is the one that gets std::cin
      if (mpi.rank() == 0) {
        std::cin >> overall_start >> overall_finish;  // Else ask for the data
        // Send messages to every other process
        for (int destination = 1; destination < mpi.size(); ++destination) {
          // MPI can only send continuous data, i.e. arrays. If it sends
          // pointers it won't be valid at destination. ANything with
          // virtual or polymorphic behavior will not work in dest
          // int MPI_Send(const void* buff, int count, MPI_Datatype datatype
          // , int dest, int tag, MPI_Comm comm)
          if (MPI_Send(&overall_start, /*count*/ 1, MPI_INT, destination
              , /*tag*/ 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
            throw Mpi::Error("could not send start", mpi);
          }
          if (MPI_Send(&overall_finish, /*count*/ 1, MPI_INT, destination
              , /*tag*/ 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
            throw Mpi::Error("could not send end", mpi);
          }
        }
      } else {
        // If is not process 0, it only has to receive the info
        // int MPI_Recv(void* buff, int count, MPI_Datatype datatype
        // , int source, int tag, MPI_Comm comm, MPI_Status status)
        if (MPI_Recv(&overall_start, /*count*/ 1, MPI_INT
            , /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)
            != MPI_SUCCESS) {
          throw Mpi::Error("could not receive start", mpi);
        }
        if (MPI_Recv(&overall_finish, /*count*/ 1, MPI_INT
            , /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)
            != MPI_SUCCESS) {
          throw Mpi::Error("could not receive finish", mpi);
        }
      }
    }
    const int process_start = calculate_start(mpi.rank(), overall_finish
        , mpi.size(), overall_start);
    const int process_finish = calculate_finish(mpi.rank(), overall_finish
        , mpi.size(), overall_start);
    const int process_size = process_finish - process_start;

    std::cout << mpi.getHostname() << ':' << mpi.getProcessNumber()
        << ": range [" << process_start << ", " << process_finish
        << "[ size " << process_size << std::endl;

    #pragma omp parallel default(none) \
      shared(process_start, process_finish, mpi, std::cout)
    {  // NOLINT(whitespace/braces)
      int thread_start = -1;
      int thread_finish = -1;

      #pragma omp for
      for (int index = process_start; index < process_finish; ++index) {
        // do_task
        if (thread_start == -1) {
          thread_start = index;
        }
        thread_finish = index + 1;
      }
      const int thread_size = thread_finish - thread_start;

      #pragma omp critical(print)
      std::cout << '\t' << mpi.getHostname() << ':' << mpi.getProcessNumber()
          << '.' << omp_get_thread_num() << ": range [" << thread_start
          << ", " << thread_finish << "[ size " << thread_size << std::endl;
    }
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
  return 0;
}

int calculate_start(int rank, int end, int workers, int begin) {
  const int range = end - begin;
  return begin + rank * (range / workers) + std::min(rank, range % workers);
}

int calculate_finish(int rank, int end, int workers, int begin) {
  return calculate_start(rank + 1, end, workers, begin);
}
