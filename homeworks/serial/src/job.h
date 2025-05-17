// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef JOB_H
#define JOB_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "errors.h"
#include "plate.h"

/** @brief Maximum file name size. */
#define MAX_FILE_NAME_SIZE 256

/** @brief Folder name for report files. */
#define REPORTS_DIRECTORY "reports"

#define REPORT_HEADER "PLATE NAME\tINTERVAL\tTHERMAL DIFF\tCELLS DIM" \
                     "\tEPSILON\tSIM STATES\tSIM TIME\n"

/**
 * @brief Carries out simulation of each plate in an indicated job.
 * 
 * @param job_file_path path of job to simulate
 * @param thread_count amount of threads used to simulate
 * @return Success or failure of procedure
 */
int simulate(char* job_file_path, uint64_t thread_count);


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
int report_plate_result(FILE* results_file, plate_t* plate);

/**
 * @brief Calls upon common functions to build the report file's paths.
 * @param job_file_path Path of the job
 * @return Report file path built.
 */
char* build_report_file_path(const char* job_file_path);

#endif  // JOB_H
