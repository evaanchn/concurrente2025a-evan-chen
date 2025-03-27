// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef PLATE_H
#define PLATE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "plate_matrix.h"

typedef struct plate {
  char* file_name;
  uint64_t cells_dimension;
  uint64_t interval_duration;
  double thermal_diffusivity;
  double epsilon;
  plate_matrix_t * plate_matrix;
  uint64_t k_states;
} plate_t;

void set_plate_matrix(plate_t* plate);

bool update_plate(plate_t* plate);

void update_plate_file(plate_t* plate);

#endif  // PLATE_H
