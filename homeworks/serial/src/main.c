// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "job.h"

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

  if (argc < 2) {
    // print "Error: No job file specified"
    perror("ERROR: No job file specified\n");
    return ERR_NO_JOB_FILE;
  }  // else if (argc == 2) {
  //   // Taken from hello_w example, calls sysconf to get available cores.
  //   thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  // } else {
  //   // Amount of threads to use specified in command
  //   sscanf(argv[2], "%" SCNu64, &thread_count);
  // }

  // Record start time
  struct timespec start_time, finish_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  int error = simulate(argv[1], thread_count);

  // Record end time
  clock_gettime(CLOCK_MONOTONIC, &finish_time);

  // Set elapsed time
  double elapsed_time = get_elapsed_seconds(&start_time, &finish_time);

  // Report elapsed time
  printf("Completed job in: %.9lfs\n", elapsed_time);

  return error;
}
