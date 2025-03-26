// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef PLATE_MATRIX_H
#define PLATE_MATRIX_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

// #include "plate_matrix.h"

typedef struct plate_matrix {
  uint64_t rows;
  uint64_t cols;
  double** matrix;
  double** auxiliary_matrix;
} plate_matrix_t;

plate_matrix_t* init_place_matrix(uint64_t rows, uint64_t cols);

void set_auxiliary(plate_matrix_t* plate_matrix);

void update_cell(plate_matrix_t* plate_matrix, uint64_t rows, uint64_t cols,
    double mult_constant);


void destroy_matrices(plate_matrix_t* plate_matrix);

double** create_double_matrix(const uint64_t rows, const uint64_t cols);

void destroy_double_matrix(double** matrix, const uint64_t rows);

#endif  // PLATE_MATRIX_H
