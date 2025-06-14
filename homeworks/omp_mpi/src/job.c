// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"
#include <omp.h>

/// @brief Implements static map by blocks formula, adapted for processes
/// @param rank Process number
/// @param work_amount Amount of work to distribute
/// @param workers Process count
/// @return Index of start
uint64_t calculate_start(int rank, int work_amount, int workers);

/// @brief Calculates finish index with formula for static map by blocks
/// @see calculate_start
/// @return Index of finish (exclusive)
uint64_t calculate_finish(int rank, int work_amount, int workers);

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

  // Record start time
  struct timespec start_time, finish_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  // Process the plates
  error = process_plates(job, &mpi, thread_count);
  if (error != EXIT_SUCCESS) return error;

  // Record end time
  clock_gettime(CLOCK_MONOTONIC, &finish_time);

  // Set elapsed time
  double elapsed_time = get_elapsed_seconds(&start_time, &finish_time);

  // Report elapsed time
  printf("[PROCESS %d] completed share in: %.9lfs\n", mpi.process_number
      , elapsed_time);

  // Report final results of the simulation
  report_results(job, &mpi);
  // Deallocation and mpi finalization
  destroy_job(job);
  mpiwrapper_finalize();
  return EXIT_SUCCESS;
}



int process_plates(job_t* job, mpi_t* mpi, uint64_t thread_count) {
  job->plates_start = calculate_start(mpi->process_number, job->plates_count
      , mpi->process_count);
  job->plates_finish = calculate_finish(mpi->process_number, job->plates_count
      , mpi->process_count);
  // For each plate stored
  for (size_t plate_number = job->plates_start; plate_number
      < job->plates_finish; ++plate_number) {
    // Get current plate
    plate_t* curr_plate = job->plates[plate_number];

    // Create plate's plate matrix: read plate file and store temperatures
    int error = set_plate_matrix(curr_plate, job->source_directory);
    if (error != EXIT_SUCCESS) {
      destroy_job(job);
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
    printf("[PROCESS %d] Equilibrated plate %zu in: %.9lfs\n"
        , mpi->process_number, plate_number, elapsed_time);

    clean_plate(job, plate_number);
  }
  return EXIT_SUCCESS;
}

uint64_t calculate_start(int rank, int work_amount, int workers) {
  // Add the residue if thread number exceeds it
  size_t added = rank < work_amount % workers ? rank : work_amount % workers;
  return rank * (work_amount / workers) + added;
}

uint64_t calculate_finish(int rank, int work_amount, int workers) {
  return calculate_start(rank + 1, work_amount, workers);
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


int report_results(job_t* job, mpi_t* mpi) {
  // If process is not the first, send results only
  if (mpi->process_number != FIRST_PROCESS) {
    return send_results(job);
  }

  // If not, it's the first one and it must report into report file
  int error = EXIT_SUCCESS;
  char* results_file_path = build_report_file_path(job);

  if (!results_file_path) {
    perror("Error: Results file path could not be built");
    return ERR_RESULTS_FILE_PATH;
  }

  // Open results file for writing
  FILE* results_file = fopen(results_file_path, "w");

  if (results_file) {
    // For every process simulating
    for (int process_number = FIRST_PROCESS; process_number
        < mpi->process_count; ++process_number) {
      // Calculate their start and finish
      uint64_t process_start = calculate_start(process_number, job->plates_count
      , mpi->process_count);
      uint64_t process_finish = calculate_finish(process_number
          , job->plates_count, mpi->process_count);
      // Iterate through interval
      for (uint64_t plate_number = process_start; plate_number < process_finish;
          ++plate_number) {
        // If current process is the first one
        if (process_number == FIRST_PROCESS) {
          // It has the data of the simulation, so it can just write it
          write_result(job, results_file, plate_number
              , job->plates[plate_number]->k_states);
        } else {
          // Otherwise, it waits for the current process to send the states
          uint64_t k_states = 0;
          mpiwrapper_recv(&k_states, 1, MPI_INT, process_number, 0);
          write_result(job, results_file, plate_number, k_states);
        }
      }
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

int send_results(job_t* job) {
  int error = EXIT_SUCCESS;
  // From worked plates in job, from start to finish
  // , send results (iterations) of simulation to 1st process 
  for (size_t plate_number = job->plates_start; plate_number
        < job->plates_finish; ++plate_number) {
    // Obtain information to send
    uint64_t k_states = job->plates[plate_number]->k_states;
    error = mpiwrapper_send(&k_states, /*count*/ 1, MPI_INT
        , /*dest*/ FIRST_PROCESS, /*tag*/ 0);  // Send states of plate
    if (error != MPI_SUCCESS) {
      return error;
    }
  }
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

void write_result(job_t* job, FILE* results_file, int plate_number
    , uint64_t k_states) {
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
      k_states,
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
