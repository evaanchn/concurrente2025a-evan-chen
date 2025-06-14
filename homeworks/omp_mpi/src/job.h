// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef JOB_H
#define JOB_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif  // _DEFAULT_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "errors.h"
#include "plate.h"
#include "threads.h"

#include "mpi_wrapper.h"

/** @brief Initial capacity for plates allocation. */
#define STARTING_CAPACITY 50

/** @brief Maximum file name size. */
#define MAX_FILE_NAME_SIZE 256

/** @brief Folder name for report files. */
#define REPORTS_DIRECTORY "reports"

/** @brief First process ID */
#define FIRST_PROCESS 0

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
 * @brief Carries out simulation of each plate in an indicated job.
 * 
 * @param job_file_path path of job to simulate
 * @param thread_count amount of threads used to simulate
 * @return Success or failure of procedure
 */
int simulate(char* job_file_path, uint64_t thread_count);

/**
 * @brief First process' is job master, delegates work to workers in this 
 * procedure, until all plates are simulated
 * 
 * @param job Job with info for master to distribute work
 * @param mpi Mpi struct with process info
 * @return Success or failure of procedure
 */
int job_master_process(job_t* job, mpi_t* mpi);

/// @brief Iterates through worker processes' IDs and signals each to stop.
/// @see job_master_process
int job_master_stop_workers(job_t* job, mpi_t* mpi);

/**
 * @brief Receives plate indexes to process and report back to master.
 * 
 * @param job Job with info for worker to simulate plate
 * @param thread_count Amount of threads used to simulate the plate
 * @return Success or failure of procedure
 */
int job_worker_process(job_t* job, uint64_t thread_count);

/**
 * @brief Loops through all of the plates recorded to simulate.
 * 
 * @param job current working job
 * @param thread_count Amount of threads available for use
 * @return Success or failure of processing
 */
int process_plates(job_t* job, uint64_t thread_count);

/**
 * @brief Simulates heat transfer of one plate and reports duration
 * 
 * @param job current working job
 * @param plate_number Number of plate to process
 * @param thread_count Amount of threads available for simulation
 * @return Success or failure of processing
 */
int process_plate(job_t* job, uint64_t plate_number, uint64_t thread_count);

/// @brief Carries out recording of updated plate and freeing of memory.
/// @see equilibrate_plates
/// @return if clean up if successful
int clean_plate(job_t* job, size_t plate_number);

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

/// @brief Writes the results of a plate's simulation into a file
/// @param job Pointer to job struct with the plates
/// @param results_file Results file to report to
/// @param plate_number Number of plate to extract data from
void write_result(job_t* job, FILE* results_file, int plate_number);

/**
 * @brief Formats a time value into a human-readable string.
 * @param seconds Time in seconds.
 * @param text Buffer to store formatted time.
 * @param capacity Size of the buffer.
 * @return Pointer to formatted time string.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity);


#endif  // JOB_H
