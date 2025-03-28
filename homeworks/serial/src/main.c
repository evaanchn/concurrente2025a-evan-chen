// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "simulation.h"

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
    return 11;
  } else if (argc < 4) {
    perror("ERROR: No source directory or output directory specified\n");
    return 12;
  } // else if (argc == 4) {
  //   // Taken from hello_w example, calls sysconf to get available cores.
  //   thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  // } else {
  //   // Amount of threads to use specified in command
  //   sscanf(argv[2], "%" SCNu64, &thread_count);
  // }

  int error = simulate(argv[1], argv[2], argv[3], thread_count);
  return error;
}
