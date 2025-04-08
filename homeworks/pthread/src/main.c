// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "job.h"

int analyze_arguments(int argc, char* argv[], uint64_t* thread_count);

/**
 * @brief Processes execution command to set thread count and 
 *        manage if job file was specified. Calls simulate.
 * @param argc, argv: argc (int)- how many arguments were passed
 *                    argv (char*) - the array of arguments 
 * @return Status code to the operating system, 0 means success.
 */
int main(int argc, char* argv[]) {
  // Amount of threads to use in simulation (IMPLEMENTED FOR HW2)
  uint64_t thread_count = 1;

  int error = analyze_arguments(argc, argv, &thread_count);

  if (error == EXIT_SUCCESS) error = simulate(argv[1], thread_count);

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
        "usage: bin/pthread job_file_path thread_count (count optional)\n");
    error = ERR_NO_JOB_FILE;
  }
  return error;
}
