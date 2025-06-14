// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "mpi_wrapper.h"

// Initializes the MPI Wrapper
int mpiwrapper_init(mpi_t* wrapper) {
  // Initialize wrapper attributes
  wrapper->process_number = -1;
  wrapper->process_count = -1;
  // Set process number
  if (MPI_Comm_rank(MPI_COMM_WORLD, &wrapper->process_number) != MPI_SUCCESS) {
    perror("Error: could not get MPI rank");
    return ERR_SET_PROCESS_NUMBER;
  }
  // Set process count
  if (MPI_Comm_size(MPI_COMM_WORLD, &wrapper->process_count) != MPI_SUCCESS) {
    perror("Error: could not get MPI size");
    return ERR_SET_PROCESS_COUNT;
  }
  return EXIT_SUCCESS;
}

// Wrapper for send
int mpiwrapper_send(void* data, int count, MPI_Datatype datatype, int dest) {
  if (MPI_Send(data, count, datatype, dest, /*tag*/ 0, MPI_COMM_WORLD)
      != MPI_SUCCESS) {
    perror("Error: could not get send data");
    return ERR_MPI_SEND;
  }
  return EXIT_SUCCESS;
}

// Wrapper for receive
int mpiwrapper_recv(void* data, const int capacity, MPI_Datatype datatype
    , int source) {
  if (MPI_Recv(data, capacity, datatype, source, /*tag*/ 0, MPI_COMM_WORLD
      , MPI_STATUS_IGNORE) != MPI_SUCCESS) {
    perror("Error: could not get sent data");
    return ERR_MPI_RECV;
  }
  return EXIT_SUCCESS;
}

// Finalizes MPI (not strictly part of wrapper, but often tied to it)
void mpiwrapper_finalize() {
  MPI_Finalize();
}
