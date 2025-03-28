// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef JOB_H
#define JOB_H

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "plate.h"

typedef struct job {
  char* file_name;
  char* source_directory;
  char* output_directory;
  size_t plates_count;
  size_t plates_capacity;
  plate_t** plates;
} job_t;

job_t* init_job(char* job_file_name, char* source_dir, char* output_dir);

int set_job(job_t* job);

int check_capacity(job_t* job);

int expand_job(job_t* job);

void destroy_job(job_t* job);

char* format_time(const time_t seconds, char* text, const size_t capacity);

int report_results(job_t* job);

#endif  // JOB_H
