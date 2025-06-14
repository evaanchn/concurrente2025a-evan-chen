// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"
#include <omp.h>

// ***[JOB RELATED]***

job_t* init_job(char* job_file_name) {
  // Allocate memory for a new job structure
  job_t* job = (job_t*) calloc(1, sizeof(job_t));

  if (job) {
    // Initialize job properties
    job->file_name = job_file_name;
    job->source_directory = extract_directory(job_file_name);
    job->plates_count = 0;
    job->plates_capacity = STARTING_CAPACITY;
    // Allocate memory for plates array
    job->plates = (plate_t**) calloc(job->plates_capacity, sizeof(plate_t*));

    // Check if plates memory allocation failed
    if (!job->plates) {
      perror("Error: Memory for plates could not be allocated\n");
      destroy_job(job);
      return NULL;
    }
  } else {
    perror("Error: Memory for job could not be allocated\n");
    return NULL;
  }

  return job;
}

int set_job(job_t* job) {
  // Open the job file for reading
  FILE* job_file = fopen(job->file_name, "rt");

  if (!job_file) {
    perror("Error: Job file could not be opened\n");
    destroy_job(job);
    return ERR_JOB_FILE_NOT_FOUND;
  }
  // Variables to store plate properties
  char plate_file_name[MAX_FILE_NAME_SIZE] = "\0";
  uint64_t interval_duration = 0;
  double thermal_diffusivity = 0;
  double cells_dimension = 0;
  double epsilon = 0;

  // Read and parse each line from the job file
  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lg\t%lg\t%lg\n",
    plate_file_name, &interval_duration, &thermal_diffusivity,
    &cells_dimension, &epsilon) == 5) {
    // Allocate memory for a new plate
    plate_t* curr_plate = (plate_t*) calloc(1, sizeof(plate_t));

    if (!curr_plate) {
      perror("Error: Memory for new plate could not be allocated\n");
      destroy_job(job);
      return ERR_PLATE_ALLOC;
    }

    // Allocate memory for plate file name
    curr_plate->file_name = (char*) calloc(1, strlen(plate_file_name) + 1);
    if (!curr_plate->file_name) {
      perror("Error: Memory for plate file name could not be allocated\n");
      free(curr_plate);  // Free allocated memory before returning
      destroy_job(job);
      return ERR_PLATE_FILE_NAME_ALLOC;
    }

    // Copy file name and set plate properties
    snprintf(curr_plate->file_name, strlen(plate_file_name) + 1, "%s",
        plate_file_name);

    curr_plate->interval_duration = interval_duration;
    curr_plate->thermal_diffusivity = thermal_diffusivity;
    curr_plate->cells_dimension = cells_dimension;
    curr_plate->epsilon = epsilon;

    // Add plate to job's plates array
    job->plates[job->plates_count] = curr_plate;
    job->plates_count += 1;

    // Check if capacity needs to be expanded
    if (check_capacity(job) != EXIT_SUCCESS) {
      perror("Error: Could not expand plates array");
      free(curr_plate);  // Free allocated memory before returning
      destroy_job(job);
      return ERR_JOB_EXPANSION;
    }
  }

  fclose(job_file);
  return EXIT_SUCCESS;
}


int check_capacity(job_t* job) {
  // If we've reached capacity for storage
  if (job->plates_count == job->plates_capacity) {
    // Call array expanding function
    return expand_job(job);
  }
  return EXIT_SUCCESS;
}



int expand_job(job_t* job) {
  job->plates_capacity *= 2;

  // Use realloc to move plates to bigger array
  plate_t** temp = (plate_t**) realloc(job->plates, sizeof(plate_t*)
      * job->plates_capacity);

  // If allocation is unsuccessful, destroy job and return failure
  if (!temp) {
    destroy_job(job);
    return EXIT_FAILURE;
  }

  job->plates = temp;
  return EXIT_SUCCESS;
}




void destroy_job(job_t* job) {
  // Free memory allocated for each plate
  for (size_t i = 0; i < job->plates_count; ++i) {
    free(job->plates[i]->file_name);
    free(job->plates[i]);
  }
  // Free memory allocated for plates array
  free(job->plates);
  // Free job properties
  free(job->source_directory);
  free(job);
}

// ***[SIMULATION RELATED]***

int simulate(char* job_file_path, uint64_t thread_count) {
  mpi_t mpi;
  int error = mpiwrapper_init(&mpi);
  if (error != EXIT_SUCCESS) return error;

  // Create job struct
  job_t* job = init_job(job_file_path);
  if (!job) return ERR_JOB_INIT;

  // Set the struct with necessary information
  error = set_job(job);
  if (error != EXIT_SUCCESS) return error;

  // If process is first
  if (mpi.process_number == FIRST_PROCESS) {
    // Record start time
    struct timespec start_time, finish_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
  
    // If there is more than one process involved
    if (mpi.process_count > 1) {
      error = job_master_process(job, &mpi);
      //error = process_plates(job, thread_count);
    } else {
      // Process plates by itself
      error = process_plates(job, thread_count);
    }
  
    if (error != EXIT_SUCCESS) {
        destroy_job(job);
        mpiwrapper_finalize();
        return error;
    }

    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &finish_time);
  
    // Set elapsed time
    double elapsed_time = get_elapsed_seconds(&start_time, &finish_time);
  
    // Report elapsed time
    printf("Completed job in: %.9lfs\n", elapsed_time);
  
    // Report final results of the simulation
    report_results(job);
  } else {
    // If process is not master, then run worker procedure
    job_worker_process(job, thread_count);
  }

  printf("[PROCESS %d] done\n", mpi.process_number);
  // Deallocation and mpi finalization
  destroy_job(job);
  mpiwrapper_finalize();
  return EXIT_SUCCESS;
}

int job_master_process(job_t* job, mpi_t* mpi) {
  int error = EXIT_SUCCESS;
  // Keep record of current index
  int current_plate_idx = 0;
  // Excluding first, other processes are workers
  int available_workers = mpi->process_count - 1;
  // Initial distribution of work, one plate for each available worker
  for (int process_number = FIRST_PROCESS + 1;
      process_number < mpi->process_count; ++process_number) {
    // Send the index of the plate to work in
    error = mpiwrapper_send(&current_plate_idx, 1, MPI_INT, process_number);
    if (error != EXIT_SUCCESS) return error;
    ++current_plate_idx;  // Move on to next plate
    --available_workers;  // One less available worker
  }
  while (true) {
    MPI_Status status;
    int received_plate_idx = -1;
    // Wait for any process to finish their plate and send the index
    if (MPI_Recv(&received_plate_idx, 1, MPI_INT, MPI_ANY_SOURCE, 0
        , MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
      perror("Error: could not get plate index from other processes");
      return ERR_MPI_RECV;
    }
    uint64_t k_states = 0;
    // Obtain k_states simulated from source
    error = mpiwrapper_recv(&k_states, 1, MPI_INT, status.MPI_SOURCE);
    if (error != EXIT_SUCCESS) return error;

    // Update in own record
    if (received_plate_idx < job->plates_count)
      job->plates[received_plate_idx]->k_states = k_states;
    ++available_workers;  // One worker became available

    // If plates have not been fully processed
    if (current_plate_idx < job->plates_count) {
      // Send the next plate index back to sender
      error = mpiwrapper_send(&current_plate_idx, 1, MPI_INT
          , status.MPI_SOURCE);
      ++current_plate_idx;  // Move on to next plate
      --available_workers;  // One worker got sent to do work
    } else {
      // Check if all workers finished and are on standby again
      if (available_workers == mpi->process_count - 1) break;
    }
  }

  // Stop workers
  error = job_master_stop_workers(job, mpi);
  return error;
}

int job_master_stop_workers(job_t* job, mpi_t* mpi) {
  int error = EXIT_SUCCESS;
  // Send stop signals to the other processes
  for (int process_number = FIRST_PROCESS + 1;
      process_number < mpi->process_count; ++process_number) {
    // Sending count indicates it has to end
    error = mpiwrapper_send(&job->plates_count, 1, MPI_INT, process_number);
    if (error != EXIT_SUCCESS) return error;
  }
  return error;
}

int job_worker_process(job_t* job, uint64_t thread_count) {
  int error = EXIT_SUCCESS;
  // Keep waiting for plate to be assigned
  while (true) {
    // Obtain index to work on
    int working_plate_idx = 0;
    error = mpiwrapper_recv(&working_plate_idx, 1, MPI_INT, FIRST_PROCESS);
    // If receive failed or the index sent is one out of range, return error
    if (error != EXIT_SUCCESS || working_plate_idx >= job->plates_count) break;

    // Process the plate
    process_plate(job, working_plate_idx, thread_count);

    // First send index so the master process knows which one it is
    error = mpiwrapper_send(&working_plate_idx, 1, MPI_INT, FIRST_PROCESS);
    if (error != EXIT_SUCCESS) break;

    // Send k states simulated for the assigned plate.
    uint64_t k_states = job->plates[working_plate_idx]->k_states;
    error = mpiwrapper_send(&k_states, 1, MPI_INT, FIRST_PROCESS);
    if (error != EXIT_SUCCESS) break;
  }
  return error;
}

int process_plates(job_t* job, uint64_t thread_count) {
  // Process every single plate registered. Do this when only one process is
  // running
  for (uint64_t plate_number = 0; plate_number < job->plates_count;
       ++plate_number) {
    process_plate(job, plate_number, thread_count);
  }
  return EXIT_SUCCESS;
}

int process_plate(job_t* job, uint64_t plate_number, uint64_t thread_count) {
  int error = EXIT_SUCCESS;
  // Get current plate
  plate_t* curr_plate = job->plates[plate_number];

  // Create plate's plate matrix: read plate file and store temperatures
  error = set_plate_matrix(curr_plate, job->source_directory);
  if (error != EXIT_SUCCESS) {
    destroy_plate_matrix(curr_plate->plate_matrix);
    return error;
  }

  // Record start time
  struct timespec start_time, finish_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  equilibrate_plate(curr_plate, thread_count);

  // Record end time
  clock_gettime(CLOCK_MONOTONIC, &finish_time);

  // Set elapsed time
  double elapsed_time = get_elapsed_seconds(&start_time, &finish_time);

  // Report elapsed time
  printf("Equilibrated plate %zu in: %.9lfs\n", plate_number, elapsed_time);

  clean_plate(job, plate_number);
  return error;
}


int clean_plate(job_t* job, size_t plate_number) {
  plate_t* curr_plate = job->plates[plate_number];
  // Create an updated plate file with final temperatures
  int error = update_plate_file(curr_plate, job->source_directory);
  if (error != EXIT_SUCCESS) {
    destroy_job(job);
    return error;
  }

  // Deallocate memory so other plates have space for their matrices
  destroy_plate_matrix(curr_plate->plate_matrix);
  return EXIT_SUCCESS;
}


int report_results(job_t* job) {
  int error = EXIT_SUCCESS;
  char* results_file_path = build_report_file_path(job);

  if (!results_file_path) {
    perror("Error: Results file path could not be built");
    return ERR_RESULTS_FILE_PATH;
  }

  // Open results file for writing
  FILE* results_file = fopen(results_file_path, "w");

  if (results_file) {
    for (size_t plate_number = 0; plate_number < job->plates_count;
       ++plate_number) {
      write_result(job, results_file, plate_number);
    }
    
    printf("Results stored in: %s\n", results_file_path);
    fclose(results_file);
  } else {
    perror("Error: Could not open results file");
    error = ERR_OPEN_RESULTS_FILE;
  }

  free(results_file_path);
  return error;
}

char* build_report_file_path(job_t* job) {
  // Extract file name from job file path
  char* file_name = extract_file_name(job->file_name);

  if (!file_name) return NULL;

  // Modify file extension to .tsv
  char* file_name_tsv = modify_extension(file_name, "tsv");

  if (!file_name_tsv) {
    free(file_name);
    return NULL;
  }

  // Build results file path
  char* results_file_path = build_file_path(REPORTS_DIRECTORY, file_name_tsv);

  free(file_name);
  free(file_name_tsv);
  return results_file_path;
}

void write_result(job_t* job, FILE* results_file, int plate_number) {
  plate_t* plate = job->plates[plate_number];  // Obtain plate
  // Calculate the time
  time_t simulated_seconds = plate->k_states * plate->interval_duration;
  char formatted_time[50];
  format_time(simulated_seconds, formatted_time, 50);
  // Print the results into the file
  fprintf(results_file, "%-10s\t%9" PRIu64 "\t%8.6lg\t%6.6lg\t%6.6lg\t%6"
      PRIu64 "\t%-48s\n",
      plate->file_name,
      plate->interval_duration,
      plate->thermal_diffusivity,
      plate->cells_dimension,
      plate->epsilon,
      plate->k_states,
      formatted_time);
}

// CODE PROVIDED IN HOMEWORK DETAILS, MODIFIED TO APPEAL TO LINTER
// Modifications credits to Albin Monge (gmtime_r part)
// Return parameter text must have at least 48 chars (YYYY/MM/DD hh:mm:ss)
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  // Convert seconds to UTC time
  struct tm gmt_r;  // For gmtime_r usage, which is threadsafe
  struct tm* gmt = gmtime_r(&seconds, &gmt_r);
  // Format time as string
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year
    - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
    gmt->tm_min, gmt->tm_sec);
  return text;
}
