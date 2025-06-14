// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "mpi_wrapper.h"

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

int mpiwrapper_send(void* data, int count, MPI_Datatype datatype, int dest) {
  // Defaults with MPI_COMM_WORLD for comm type, tag 0
  if (MPI_Send(data, count, datatype, dest, /*tag*/ 0, MPI_COMM_WORLD)
      != MPI_SUCCESS) {
    // Report error if unsuccessful
    perror("Error: could not get send data");
    return ERR_MPI_SEND;
  }
  return EXIT_SUCCESS;
}

int mpiwrapper_recv(void* data, const int capacity, MPI_Datatype datatype
    , int source) {
  // Default tag 0, global communication between processes, ignore status
  if (MPI_Recv(data, capacity, datatype, source, /*tag*/ 0, MPI_COMM_WORLD
      , MPI_STATUS_IGNORE) != MPI_SUCCESS) {
    // Report error if any
    perror("Error: could not get sent data");
    return ERR_MPI_RECV;
  }
  return EXIT_SUCCESS;
}

void mpiwrapper_finalize() {
  MPI_Finalize();  // Calls for MPI to finalize
}
