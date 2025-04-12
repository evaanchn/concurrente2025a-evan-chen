// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// Simulates a producer and a consumer that share a unbounded buffer

#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"

int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  // Create simulation
  simulation_t* simulation = simulation_create();
  // If successful
  if (simulation) {
    // Run simulation
    error = simulation_run(simulation, argc, argv);
    // Destroy simulation
    simulation_destroy(simulation);
  } else {
    fprintf(stderr, "error: could not allocate simulation\n");
    error = EXIT_FAILURE;
  }
  return error;
}