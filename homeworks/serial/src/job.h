// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef JOB_H
#define JOB_H

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "plate.h"

/** @brief Initial capacity for plates allocation. */
#define STARTING_CAPACITY 10

/** @brief Maximum file name size. */
#define MAX_FILE_NAME_SIZE 256

/** @brief Folder name for report files. */
#define REPORTS_DIRECTORY "reports"

/**
 * @struct job_t
 * @brief Represents a job containing multiple plates.
 */
typedef struct {
    char* file_name;        /**< Job file name. */
    char* source_directory; /**< Directory containing job files. */
    size_t plates_count;    /**< Number of plates. */
    size_t plates_capacity; /**< Capacity of plates array. */
    plate_t** plates;       /**< Array of plate pointers. */
} job_t;

/**
 * @brief Initializes a job from a given job file name.
 * @param job_file_name Name of the job file.
 * @return Pointer to the initialized job, or NULL on failure.
 */
job_t* init_job(char* job_file_name);

/**
 * @brief Sets up a job by reading plates from a file.
 * @param job Pointer to the job structure.
 * @return EXIT_SUCCESS on success, error code on failure.
 */
int set_job(job_t* job);

/**
 * @brief Checks if job needs expansion and expands if necessary.
 * @param job Pointer to the job structure.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int check_capacity(job_t* job);

/**
 * @brief Expands the job's plate capacity.
 * @param job Pointer to the job structure.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int expand_job(job_t* job);

/**
 * @brief Destroys a job and frees allocated memory.
 * @param job Pointer to the job structure.
 */
void destroy_job(job_t* job);

/**
 * @brief Formats a time value into a human-readable string.
 * @param seconds Time in seconds.
 * @param text Buffer to store formatted time.
 * @param capacity Size of the buffer.
 * @return Pointer to formatted time string.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity);

/**
 * @brief Generates a report file from the job's simulation results.
 * @param job Pointer to the job structure.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int report_results(job_t* job);

/**
 * @brief Calls upon common functions to build the report file's paths.
 * @param job Pointer to the job structure.
 * @return Report file path built.
 */
char* build_report_file_path(job_t* job);

#endif  // JOB_H
