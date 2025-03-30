// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "simulation.h"

int simulate(char* job_file_path, uint64_t thread_count) {
  if (thread_count > 1) {
    printf("Concurrent solution yet to be developed\n");
  }

  // Create job struct
  job_t* job = init_job(job_file_path);
  if (!job) return 21;

  // Set the struct with necessary information
  if (set_job(job) != EXIT_SUCCESS) {
    destroy_job(job);
    return 22;
  }

  // For each plate stored
  for (size_t plate_number = 0; plate_number < job->plates_count;
        ++plate_number) {
    // Get current plate
    plate_t* curr_plate = job->plates[plate_number];

    // Create plate's plate matrix: read plate file and store temperatures
    if (set_plate_matrix(curr_plate, job->source_directory) != EXIT_SUCCESS) {
      destroy_job(job);
      return 24;
    }

    // Loop to simulate the plate's changes in temperature
    uint64_t k_states = 0;
    bool reached_equilibrium = false;

    //  while not reached_equilibrium do
    while (!reached_equilibrium) {
      //  update_plate(plate)
      reached_equilibrium = update_plate(curr_plate);

      ++k_states;
    }  //  end while

    // Store k, number of states iterated until equilibrium, in plate
    curr_plate->k_states = k_states;

    // Create an updated plate file with final temperatures
    if (update_plate_file(curr_plate, job->source_directory) != EXIT_SUCCESS) {
      destroy_job(job);
      return 25;
    }

    // Deallocate memory so other plates have space for their matrices
    destroy_plate_matrix(curr_plate->plate_matrix);
  }

  // Report final results of the simulation
  report_results(job);
  destroy_job(job);

  return EXIT_SUCCESS;
}

