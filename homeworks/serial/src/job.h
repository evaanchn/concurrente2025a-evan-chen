#ifndef JOB_H
#define JOB_H

#include <stdlib.h>
#include <time.h>

typedef struct job{
  char* file_name;
  size_t plates_count;
  size_t plates_capacity;
  // plate_h** plates;
} job_t;

void init_job(const char* job_file_name);

void set_job(job_t* job);

void init_plates(const size_t capacity);

void destroy_job(job_t* job);

char* format_time(const time_t seconds, char* text, const size_t capacity);

void report_results(job_t* job);

#endif // JOB_H