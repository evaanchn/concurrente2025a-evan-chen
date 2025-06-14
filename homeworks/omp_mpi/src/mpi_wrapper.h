// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "errors.h"

typedef struct {
  int process_number;  // rank
  int process_count;  // size
} mpi_t;

// Initializes the MPIWrapper
int mpiwrapper_init(mpi_t* wrapper);

// Wrapper for send
int mpiwrapper_send(void* data, int count, MPI_Datatype datatype, int dest);

// Wrapper for receive
int mpiwrapper_recv(void* data, int count, MPI_Datatype datatype, int source);

// Finalizes MPI (not strictly part of wrapper, but often tied to it)
void mpiwrapper_finalize();
