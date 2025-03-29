// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "simulation.h"

int simulate(char* job_file_path, uint64_t thread_count) {
  if (thread_count > 1) {
    printf("Concurrent solution yet to be developed\n");
  }

  // const job = init_job(file_path)
  job_t* job = init_job(job_file_path);

  if (!job) {
    return 21;
  }
  
  if (set_job(job) != EXIT_SUCCESS) {
    return 22;
  }

  // for plate_number := 0 to plate_count do
  for (size_t plate_number = 0; plate_number < job->plates_count; 
      ++plate_number) {
  
    //  mutable k_states := 0
    uint64_t k_states = 0;

    //  mutable reached_equilibrium := false
    bool reached_equilibrium = false;

    plate_t* curr_plate = job->plates[plate_number];

    char* source_directory = extract_directory(job->file_name);
    if (!source_directory) { return 23; }

    if (set_plate_matrix(curr_plate, source_directory) != EXIT_SUCCESS) {
      free(source_directory);
      destroy_job(job);
      return 24;
    }

    free(source_directory);

    //  while not reached_equilibrium do
    while (!reached_equilibrium) {
      //  update_plate(plate)
      reached_equilibrium = update_plate(curr_plate);

      ++k_states;
    }  //  end while

    curr_plate->k_states = k_states;

    if (update_plate_file(curr_plate, job->source_directory)
       != EXIT_SUCCESS) {
      destroy_job(job);
      return 23;
    }

    // Free matrices memory in case it takes up too much memory
    destroy_plate_matrix(curr_plate->plate_matrix);
  }  // end for

  report_results(job);
  destroy_job(job);

  return EXIT_SUCCESS;
}
