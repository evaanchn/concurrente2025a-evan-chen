// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef PLATE_H
#define PLATE_H

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "plate_matrix.h"

typedef struct plate {
  char file_name[20];
  uint64_t cells_dimension;
  uint64_t interval_duration;
  double thermal_diffusivity;
  double epsilon;
  plate_matrix_t * plate_matrix;
  uint64_t k_states;
} plate_t;

int set_plate_matrix(plate_t* plate, char* source_directory);

bool update_plate(plate_t* plate);

int update_plate_file(plate_t* plate);

#endif  // PLATE_H
