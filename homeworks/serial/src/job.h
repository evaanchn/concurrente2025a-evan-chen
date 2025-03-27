// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef JOB_H
#define JOB_H

#include <stdlib.h>
#include <time.h>

#include "plate.h"

typedef struct job {
  char* file_name;
  size_t plates_count;
  size_t plates_capacity;
  struct plate_h** plates;
} job_t;

job_t* init_job(const char* job_file_name);

int set_job(job_t* job);

int expand_job(job_t* job);

void destroy_job(job_t* job);

char* format_time(const time_t seconds, char* text, const size_t capacity);

void report_results(job_t* job);

#endif  // JOB_H
