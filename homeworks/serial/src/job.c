// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"

job_t* init_job(char* job_file_name) {
  job_t* job = (job_t*) malloc(sizeof(job_t));

  if (job) {
    job->file_name = job_file_name;
    job->plates_count = 0;
    job->plates_capacity = 50;
    job->plates = (plate_t**) calloc(job->plates_capacity, sizeof(plate_t*));

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
  FILE* job_file = fopen(job->file_name, "rt");

  if (!job_file) {
    perror("Error: Job file could not be opened");
    destroy_job(job);
    return 21;
  }

  char plate_file_name[20];  // Allocate enough memory to hold the string
  uint64_t interval_duration;
  double thermal_diffusivity;
  uint64_t cells_dimension;
  double epsilon;

  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lf\t%" SCNu64 "%lf\n", plate_file_name,
      &interval_duration, &thermal_diffusivity, &cells_dimension,
          &epsilon) == 5) {
    
    plate_t* curr_plate = (plate_t*) malloc(sizeof(plate_t));
    
    if (!curr_plate) {
      perror("Error: Memory for new plate could not be allocated");
      destroy_job(job);
      return 22;
    }

    curr_plate->file_name = plate_file_name;
    curr_plate->interval_duration = interval_duration;
    curr_plate->thermal_diffusivity = thermal_diffusivity;
    curr_plate->cells_dimension = cells_dimension;
    curr_plate->epsilon = epsilon;

    job->plates[job->plates_count] = curr_plate;
    job->plates_count += 1;
    
    if (check_capacity(job) != EXIT_SUCCESS) { return EXIT_FAILURE; }
  }

  fclose(job_file);
  return EXIT_SUCCESS;
}

int check_capacity(job_t* job) {
  if (job->plates_count == job->plates_capacity) {
    if (expand_job(job) != EXIT_SUCCESS) {
      perror("Error: Job could not be expanded");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int expand_job(job_t* job) {
  job->plates_capacity *= 2;
  plate_t** temp = (plate_t**)
      realloc(job->plates, sizeof(plate_t*) * job->plates_capacity);
  if (!temp) {
    destroy_job(job);
    return EXIT_FAILURE;
  }

  // If all went well
  job->plates = temp;
  return EXIT_SUCCESS;
}

void destroy_job(job_t* job) {
  assert(job);
  for (size_t i = 0; i < job->plates_count; ++i) {
    free(job->plates[i]);
  }
  free(job->plates);
  free(job);
}

// Return parameter text must have at least 48 chars (YYYY/MM/DD hh:mm:ss)
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year
    - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
    gmt->tm_min, gmt->tm_sec);
  return text;
}

int report_results(job_t* job) {
  int error = EXIT_SUCCESS;

  char* results_file_name = job->file_name;
  FILE* results_file = fopen(results_file_name, "wt");
  
  if (results_file) {
    for (size_t i = 0; i < job->plates_count; ++i) {
      plate_t* plate = job->plates[i];
      time_t simulated_seconds = plate->k_states * plate->interval_duration;
      char* formatted_time = format_time (simulated_seconds, formatted_time, 50);
      fprintf(results_file, "%s\t%" PRIu64 "\t%lf\t%" PRIu64 "%lf\t%" PRIu64 
          "\t%s\n", plate->file_name, plate->interval_duration, 
          plate->thermal_diffusivity, plate->cells_dimension, plate->epsilon, 
          plate->k_states, formatted_time);
    }
  } else {
    perror("Could not open results file");
    error = EXIT_FAILURE;
  }

  fclose(results_file);
  return error;
}
