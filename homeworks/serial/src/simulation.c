// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "simulation.h"

int simulate(char* job_file_path, uint64_t thread_count) {
  // const job = init_job(file_path)
  job_t* job = init_job(job_file_path);

  if (!job) {
    return EXIT_FAILURE;
  }

  // for plate_number := 0 to plate_count do
  for (size_t plate_number = 0; plate_number < job->plates_count; 
   ++plate_number) {
    //  mutable k_states := 0
    uint64_t k_states = 0;

    //  mutable reached_equilibrium := false
    bool reached_equilibrium = false;

    //  const curr_plate = job->plates[plate_number]
    plate_t* curr_plate = job->plates[plate_number];

    //  mutable plate_matrix := set_plate_matrix(curr_plate)
    if (set_plate_matrix(curr_plate) != EXIT_SUCCESS){
      destroy_job(job);
      return EXIT_FAILURE;
    }

    //  while not reached_equilibrium do
    while (!reached_equilibrium) {
      //  update_plate(plate)
      update_plate(curr_plate);
      
      ++k_states;
    }  //  end while

    curr_plate->k_states = k_states;

    if (update_plate_file(curr_plate) != EXIT_SUCCESS) {
      destroy_job(job);
      return EXIT_FAILURE;
    }

    // Free matrices memory
    destroy_matrices(curr_plate->plate_matrix);
  }  // end for

  report_results(job);
  destroy_job(job);
  
  return EXIT_SUCCESS;
}
