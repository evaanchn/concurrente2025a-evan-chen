// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"

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
      perror("Error: Memory for plates could not be allocated");
      destroy_job(job);
      return NULL;
    }
  } else {
    perror("Error: Memory for job could not be allocated");
    return NULL;
  }

  return job;
}

int set_job(job_t* job) {
  // Open the job file for reading
  FILE* job_file = fopen(job->file_name, "rt");

  if (!job_file) {
    perror("Error: Job file could not be opened");
    destroy_job(job);
    return 22;
  }
  // Variables to store plate properties
  char plate_file_name[MAX_FILE_NAME_SIZE] = "\0";
  uint64_t interval_duration = 0;
  double thermal_diffusivity = 0;
  uint64_t cells_dimension = 0;
  double epsilon = 0;

  // Read and parse each line from the job file
  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lg\t%" SCNu64 "%lg\n",
    plate_file_name, &interval_duration, &thermal_diffusivity,
    &cells_dimension, &epsilon) == 5) {
    // Allocate memory for a new plate
    plate_t* curr_plate = (plate_t*) calloc(1, sizeof(plate_t));

    if (!curr_plate) {
      perror("Error: Memory for new plate could not be allocated");
      destroy_job(job);
      return 23;
    }

    // Allocate memory for plate file name
    curr_plate->file_name = (char*) calloc(1, strlen(plate_file_name) + 1);
    if (!curr_plate->file_name) {
      perror("Error: Memory for plate file name could not be allocated");
      free(curr_plate);  // Free allocated memory before returning
      destroy_job(job);
      return 24;
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
    if (check_capacity(job) != EXIT_SUCCESS) return EXIT_FAILURE;
  }

  fclose(job_file);
  return EXIT_SUCCESS;
}

int check_capacity(job_t* job) {
  if (job->plates_count == job->plates_capacity) {
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



int report_results(job_t* job) {
  int error = EXIT_SUCCESS;
  char* results_file_path = build_report_file_path(job);

  if (!results_file_path) {
    perror("Error: Results file path could not be built");
    return EXIT_FAILURE;  // Could not build report file path
  }

  // Open results file for writing
  FILE* results_file = fopen(results_file_path, "w");

  if (results_file) {
    // Write each plate's data to the file
    for (size_t i = 0; i < job->plates_count; ++i) {
      plate_t* plate = job->plates[i];
      time_t simulated_seconds = plate->k_states * plate->interval_duration;
      char formatted_time[50];
      format_time(simulated_seconds, formatted_time, 50);
      fprintf(results_file, "%-10s\t%9" PRIu64 "\t%8.6lg\t%8" PRIu64
          "\t%6.6lg\t%6" PRIu64 "\t%-48s\n",
          plate->file_name,
          plate->interval_duration,
          plate->thermal_diffusivity,
          plate->cells_dimension,
          plate->epsilon,
          plate->k_states,
          formatted_time);
    }
    printf("Results stored in: %s\n", results_file_path);
    fclose(results_file);
  } else {
    perror("Could not open results file");
    error = EXIT_FAILURE;  // Add error code later
  }

  free(results_file_path);
  return error;
}



char* build_report_file_path (job_t* job) {
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
