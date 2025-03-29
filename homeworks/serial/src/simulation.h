// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef SIMULATION_H
#define SIMULATION_H

#include <inttypes.h>

#include "job.h"

int simulate(char* job_file_path, uint64_t thread_count);

#endif  // SIMULATION_H
