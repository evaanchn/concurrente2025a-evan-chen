// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "job.h"

#include "job.h"

// Opens the results file (write then append mode)
static FILE* open_results_file(char* results_file_path) {
  FILE* file = fopen(results_file_path, "w");
  if (!file) return NULL;

  fprintf(file, REPORT_HEADER);
  fclose(file);

  return fopen(results_file_path, "a");
}

// Initializes a plate from parameters and the file system
static int process_plate(FILE* results_file, char* plate_files_source,
                         char* plate_file_name, uint64_t interval_duration,
                         double thermal_diffusivity, double cells_dimension,
                         double epsilon, int plate_index) {
  int error = EXIT_SUCCESS;

  plate_t* curr_plate = (plate_t*)calloc(1, sizeof(plate_t));
  if (!curr_plate) {
    fprintf(stderr, "Error: Memory for new plate could not be allocated\n");
    return ERR_PLATE_ALLOC;
  }

  set_plate(curr_plate, plate_file_name, interval_duration,
            thermal_diffusivity, cells_dimension, epsilon);

  error = set_plate_matrix(curr_plate, plate_files_source);
  if (error != EXIT_SUCCESS) {
    free(curr_plate);
    return error;
  }

  struct timespec start_time, finish_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  equilibrate_plate(curr_plate);
  clock_gettime(CLOCK_MONOTONIC, &finish_time);

  double elapsed_time = get_elapsed_seconds(&start_time, &finish_time);
  printf("Equilibrated plate %d in: %.9lfs\n", plate_index, elapsed_time);

  update_plate_file(curr_plate, plate_files_source);
  destroy_plate_matrix(curr_plate->plate_matrix);
  report_plate_result(results_file, curr_plate);

  free(curr_plate->file_name);
  free(curr_plate);
  return EXIT_SUCCESS;
}

// Main simulation function
int simulate(char* job_file_path, uint64_t thread_count) {
  if (thread_count > 1) {
    printf("Concurrent solution yet to be developed\n");
  }

  FILE* job_file = fopen(job_file_path, "rt");
  if (!job_file) {
    fprintf(stderr, "Error: Job file could not be opened\n");
    return ERR_JOB_FILE_NOT_FOUND;
  }

  char* results_file_path = build_report_file_path(job_file_path);
  if (!results_file_path) {
    fclose(job_file);
    fprintf(stderr, "Error: Results file path could not be built\n");
    return ERR_RESULTS_FILE_PATH;
  }

  char* plate_files_source = extract_directory(job_file_path);
  if (!plate_files_source) {
    fclose(job_file);
    free(results_file_path);
    return ERR_PLATE_FILE_SOURCE;
  }

  FILE* results_file = open_results_file(results_file_path);
  if (!results_file) {
    fclose(job_file);
    free(results_file_path);
    free(plate_files_source);
    return ERR_OPEN_RESULTS_FILE;
  }

  char plate_file_name[MAX_FILE_NAME_SIZE] = "\0";
  uint64_t interval_duration = 0;
  double thermal_diffusivity = 0;
  double cells_dimension = 0;
  double epsilon = 0;

  int plate_index = 0;
  int error = EXIT_SUCCESS;

  while (fscanf(job_file, "%s\t%" SCNu64 "\t%lg\t%lg\t%lg\n",
                plate_file_name, &interval_duration, &thermal_diffusivity,
                &cells_dimension, &epsilon) == 5) {
    error = process_plate(results_file, plate_files_source,
                          plate_file_name, interval_duration,
                          thermal_diffusivity, cells_dimension,
                          epsilon, plate_index++);
    if (error != EXIT_SUCCESS) {
      break;
    }
  }

  fclose(results_file);
  fclose(job_file);
  free(results_file_path);
  free(plate_files_source);

  return error;
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

int report_plate_result(FILE* results_file, plate_t* plate) {
  time_t simulated_seconds = plate->k_states * plate->interval_duration;
  char formatted_time[50];
  format_time(simulated_seconds, formatted_time, 50);
  fprintf(results_file, "%-10s\t%9" PRIu64 "\t%8.6lg\t%6.6lg\t%6.6lg\t%6"
    PRIu64 "\t%-48s\n",
    plate->file_name,
    plate->interval_duration,
    plate->thermal_diffusivity,
    plate->cells_dimension,
    plate->epsilon,
    plate->k_states,
    formatted_time);
  return EXIT_SUCCESS;
}

char* build_report_file_path(const char* job_file_path) {
  // Extract file name from job file path
  char* file_name = extract_file_name(job_file_path);

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
