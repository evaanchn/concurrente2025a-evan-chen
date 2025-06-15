// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "job.h"
#include <mpi.h>

/**
 * @brief Checks whether arguments were valid.
 * @param argc Argument count.
 * @param argv Arguments vector.
 * @param *thread_count POinter to thread_count in main to set.
 * @return Success or failure of arguments analysis.
 */
int analyze_arguments(int argc, char* argv[], uint64_t* thread_count);

/**
 * @brief Processes execution command to set thread count and 
 *        manage if job file was specified. Calls simulate.
 * @param argc Arguments count.
 * @param argv Arguments vector.
 * @return Status code to the operating system, 0 means success.
 */
int main(int argc, char* argv[]) {
  // Initialize MPI
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    perror("Error: could not initialize MPI");
    return ERR_INIT_MPI;
  }
  // double start_time = MPI_Wtime();  // Record MPI start time

  // Assume default amount of threads first
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);

  int error = analyze_arguments(argc, argv, &thread_count);

  if (error == EXIT_SUCCESS) error = simulate(argv[1], thread_count);

  // double end_time = MPI_Wtime();  // Record MPI end time
  // printf("Elapsed time MPI: %lf seconds\n", end_time - start_time);

  mpiwrapper_finalize();

  return error;
}

int analyze_arguments(int argc, char* argv[], uint64_t* thread_count) {
  int error = EXIT_SUCCESS;
  // Must at least include job directory
  if (argc == 3) {
    if (sscanf(argv[2], "%zu", thread_count) != 1
        || *thread_count <= 0 || *thread_count > 32000) {
      // Inform usage to user
      fprintf(stderr,
        "Error: Invalid thread count (0 < thread_count <= 32000)\n");
      error = ERR_INVALID_THREAD_COUNT;
    }
  } else if (argc < 2) {
    // Inform usage to user
    fprintf(stderr,
        "usage: bin/omp_mpi job_file_path thread_count (count optional)\n");
    error = ERR_NO_JOB_FILE;
  }
  return error;
}
