// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "errors.h"

/**
 * @struct mpi_t
 * @brief Contains current process's ID (rank) and total amount of processes
 */
typedef struct {
  int process_number;  // Equivalent of rank in mpi
  int process_count;  // Equivalent of size in mpi
} mpi_t;

/**
 * @brief Stores process number and count into wrapper
 * 
 * @param wrapper Wrapper to initialize
 * @return Success or failure of initialization (ERR_SET_PROCESS_NUMBER,
 * ERR_SET_PROCESS_COUNT)
 */
int mpiwrapper_init(mpi_t* wrapper);

/**
 * @brief Sends data to specified process
 * @param data Address of data to send
 * @param count Amount of data to send
 * @param datatype Data's type, of MPI_Datatype
 * @param dest Destination process's number
 * @return EXIT_SUCCESS if successful, if not, MPI error for sending
 */
int mpiwrapper_send(void* data, int count, MPI_Datatype datatype, int dest);

/**
 * @brief Receives data from specified process
 * @param data Address of variable where data will be received
 * @param capacity Capacity of data buffer
 * @param datatype Data's type, of MPI_Datatype
 * @param source Sender's process's number
 * @return EXIT_SUCCESS if successful, if not, MPI error for sending
 */
int mpiwrapper_recv(void* data, int capacity, MPI_Datatype datatype
    , int source);

/// @brief Finalizes MPI
void mpiwrapper_finalize();
