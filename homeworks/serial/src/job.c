// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"

job_t* init_job(char* job_file_name) {
  // Allocate memory for the job structure
  job_t* job = (job_t*) malloc(sizeof(job_t));
  if (!job) {
    // Handle memory allocation failure for job
    perror("Error: Memory for job could not be allocated");
    return NULL;
  }

  // Initialize job fields
  job->file_name = job_file_name;
  job->source_directory = extract_directory(job_file_name);
  job->plates_count = 0;
  job->plates_capacity = STARTING_CAPACITY;

  // Allocate memory for plates array
  job->plates = (plate_t**) calloc(job->plates_capacity, sizeof(plate_t*));

  if (!job->plates) {
    // Handle memory allocation failure for plates
    perror("Error: Memory for plates could not be allocated");
    free(job);  // Free job memory if plates allocation fails
    return NULL;
  }

  return job;
}



int set_job(job_t* job) {
  // Open the job file
  FILE* job_file = fopen(job->file_name, "rt");
  if (!job_file) {
    // Handle error in opening job file
    perror("Error: Job file could not be opened");
    destroy_job(job);
    return 22;
  }

  char plate_file_name[MAX_FILE_NAME_SIZE];
  uint64_t interval_duration;
  double thermal_diffusivity;
  uint64_t cells_dimension;
  double epsilon;

  // Read the file line by line and initialize plates
  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lg\t%" SCNu64 "%lg\n",
    plate_file_name, &interval_duration, &thermal_diffusivity,
    &cells_dimension, &epsilon) == 5) {
    // Allocate memory for a new plate
    plate_t* curr_plate = (plate_t*) malloc(sizeof(plate_t));
    if (!curr_plate) {
      // Handle memory allocation failure for the plate
      perror("Error: Memory for new plate could not be allocated");
      fclose(job_file);  // Close file before returning
      destroy_job(job);
      return 23;
    }

    // Allocate memory for the plate's file name
    curr_plate->file_name = strdup(plate_file_name);
    if (!curr_plate->file_name) {
      // Handle memory allocation failure for the plate's file name
      perror("Error: Memory for plate file name could not be allocated");
      free(curr_plate);  // Free plate memory before returning
      fclose(job_file);
      destroy_job(job);
      return 24;
    }

    // Set plate properties
    curr_plate->interval_duration = interval_duration;
    curr_plate->thermal_diffusivity = thermal_diffusivity;
    curr_plate->cells_dimension = cells_dimension;
    curr_plate->epsilon = epsilon;

    // Add plate to the job
    job->plates[job->plates_count++] = curr_plate;

    // Check if the plates array capacity needs to be expanded
    if (check_capacity(job) != EXIT_SUCCESS) {
      fclose(job_file);
      return EXIT_FAILURE;
    }
  }

  fclose(job_file);
  return EXIT_SUCCESS;
}



int check_capacity(job_t* job) {
  // Check if plates array capacity is full and needs expansion
  if (job->plates_count == job->plates_capacity) {
    return expand_job(job);
  }
  return EXIT_SUCCESS;
}



int expand_job(job_t* job) {
  // Double the plates capacity
  job->plates_capacity *= 2;

  // Reallocate memory for plates array
  plate_t** temp = (plate_t**) realloc(job->plates,
      sizeof(plate_t*) * job->plates_capacity);

  if (!temp) {
    // Handle memory allocation failure during expansion
    destroy_job(job);
    return EXIT_FAILURE;
  }

  job->plates = temp;
  return EXIT_SUCCESS;
}

void destroy_job(job_t* job) {
  // Free each plate and its file name
  for (size_t i = 0; i < job->plates_count; ++i) {
    free(job->plates[i]->file_name);
    free(job->plates[i]);
  }
  // Free the plates array and the job object
  free(job->plates);
  free(job->source_directory);
  free(job);
}

char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime_r(&seconds);
  // Format the time into a string
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70,
    gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
  return text;
}

int report_results(job_t* job) {
  // Extract file name and modify its extension
  char* file_name = extract_file_name(job->file_name);
  if (!file_name) return EXIT_FAILURE;

  // Modify the extension so it goes from txt to tsv
  char* file_name_tsv = modify_extension(file_name, "tsv");
  if (!file_name_tsv) {
    free(file_name);
    return EXIT_FAILURE;
  }

  // Build the results file path
  char* results_file_path = build_file_path(REPORTS_DIRECTORY, file_name_tsv);
  free(file_name_tsv);
  free(file_name);

  if (!results_file_path) {
    perror("Error: Results file path could not be built");
    return EXIT_FAILURE;
  }

  // Open the results file
  FILE* results_file = fopen(results_file_path, "w");
  free(results_file_path);

  if (!results_file) {
    // Handle error in opening results file
    perror("Could not open results file");
    return EXIT_FAILURE;
  }

  // Write the plate data to the results file
  for (size_t i = 0; i < job->plates_count; ++i) {
    plate_t* plate = job->plates[i];
    time_t simulated_seconds = plate->k_states * plate->interval_duration;
    char formatted_time[50];
    format_time(simulated_seconds, formatted_time, 50);
    fprintf(results_file, "%s\t%" PRIu64 "\t%lg\t%" PRIu64
        "\t%lg\t%" PRIu64"\t%s\n",
        plate->file_name,
        plate->interval_duration,
        plate->thermal_diffusivity,
        plate->cells_dimension,
        plate->epsilon,
        plate->k_states,
        formatted_time);
  }

  fclose(results_file);
  return EXIT_SUCCESS;
}
