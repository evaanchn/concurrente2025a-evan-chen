// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef SIMULATION_H
#define SIMULATION_H

#include <inttypes.h>

#include "job.h"

/**
 * @brief Carries out simulation of each plate in an indicated job.
 * 
 * @param job_file_path path of job to simulate
 * @param thread_count amount of threads used to simulate
 * @return Success or failure of procedure
 */
int simulate(char* job_file_path, uint64_t thread_count);

#endif  // SIMULATION_H
