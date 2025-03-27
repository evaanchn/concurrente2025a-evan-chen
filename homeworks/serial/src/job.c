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

  char plate_file_name[40];  // Allocate enough memory to hold the string
  uint64_t interval_duration;
  double thermal_diffusivity;
  double epsilon;

  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lf\t%lf\n", plate_file_name,
              &interval_duration, &thermal_diffusivity, &epsilon) == 4) {

    plate_t* curr_plate = (plate_t*) malloc(sizeof(plate_t));

    if (!curr_plate) {
      perror("Error: Memory for new plate could not be allocated");
      destroy_job(job);
      return 22;
    }

    curr_plate->file_name = plate_file_name;
    curr_plate->interval_duration = interval_duration;
    curr_plate->thermal_diffusivity = thermal_diffusivity;
    curr_plate->epsilon = epsilon;

    job->plates_count += 1;

    if (job->plates_count == job->plates_capacity) {
      if (expand_job(job) != EXIT_SUCCESS) {
        perror("Error: Job could not be expanded");
        fclose(job_file);
        return 23;
      }
    }
  }

  fclose(job_file);
  return EXIT_SUCCESS;
}

int expand_job(job_t* job) {
  job->plates_capacity *= 2;
  plate_t** temp = 
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
    // TODO (Evan Chen): Verify this works
  const struct tm* gmt = gmtime(&seconds);
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year
    - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
    gmt->tm_min, gmt->tm_sec);
  return text;
}

void report_results(job_t* job) {
  // TODO (Evan Chen): Finish once format_time is ready
  char* results_file_name = job->file_name;
}
