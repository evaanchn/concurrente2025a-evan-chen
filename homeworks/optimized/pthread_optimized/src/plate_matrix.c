// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate_matrix.h"
#include <stdlib.h>
#include <stdio.h>

plate_matrix_t* init_plate_matrix(uint64_t rows, uint64_t cols) {
  // Allocate memory for the plate_matrix_t structure
  plate_matrix_t* plate_matrix =
      (plate_matrix_t*) calloc(1, sizeof(plate_matrix_t));

  if (!plate_matrix) return NULL;  // Handle allocation failure

  // Assign matrix dimensions
  plate_matrix->rows = rows;
  plate_matrix->cols = cols;

  // Allocate the main matrix
  plate_matrix->matrix = (double*) calloc(rows*cols, sizeof(double));
  if (!plate_matrix->matrix) {
    free(plate_matrix);
    return NULL;
  }

  // Allocate the auxiliary matrix
  plate_matrix->auxiliary_matrix = (double*) calloc(rows*cols, sizeof(double));
  if (!plate_matrix->auxiliary_matrix) {
    free(plate_matrix->matrix);  // Free main matrix before returning
    free(plate_matrix);
    return NULL;
  }

  return plate_matrix;
}



void init_auxiliary(plate_matrix_t* plate_matrix) {
  double* initial_matrix = plate_matrix->matrix;

  // Copy first row to auxiliary matrix
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    plate_matrix->auxiliary_matrix[/*0 * cols + */ col]
        = initial_matrix[/*0 * cols + */ col];
  }

  // Copy last row to auxiliary matrix
  uint64_t last_row = plate_matrix->rows - 1;
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    uint64_t index = last_row * plate_matrix-> cols + col;
    plate_matrix->auxiliary_matrix[index] = initial_matrix[index];
  }

  // Copy first column to auxiliary matrix
  for (uint64_t row = 1; row < last_row; ++row) {
    uint64_t index = row * plate_matrix-> cols /*+ first_col (0)*/;
    plate_matrix->auxiliary_matrix[index] = initial_matrix[index];
  }

  // Copy last column to auxiliary matrix
  uint64_t last_col = plate_matrix->cols - 1;
  for (uint64_t row = 1; row < last_row; ++row) {
    uint64_t index = row * plate_matrix-> cols + last_col;
    plate_matrix->auxiliary_matrix[index] = initial_matrix[index];
  }
}



void set_auxiliary(plate_matrix_t* plate_matrix) {
  // Temperatures at current state are in matrix, but we want them to be
  // stored in auxiliary to have space in the main matrix for new temperatures
  double* current_temperatures = plate_matrix->matrix;

  // Set matrix pointer to auxiliary's, which holds k-1 state temperatures
  // These will be overwritten
  plate_matrix->matrix = plate_matrix->auxiliary_matrix;

  // Auxiliary matrix now has the current temperatures
  // to properly make calculations
  plate_matrix->auxiliary_matrix = current_temperatures;
}



void update_cell(plate_matrix_t* plate_matrix, uint64_t row,
      uint64_t col, double mult_constant) {
  double* current_temp_matrix = plate_matrix->auxiliary_matrix;
  uint64_t accessed_cell_index = row * plate_matrix->cols + col;

  // Compute net energy change using the heat diffusion equation
  double result = -4 * current_temp_matrix[accessed_cell_index];
  // Top neighbor
  result += current_temp_matrix[(row - 1) * plate_matrix->cols + col];
  result += current_temp_matrix[accessed_cell_index + 1];  // Right neighbor
  // Bottom neighbor
  result += current_temp_matrix[(row + 1) * plate_matrix->cols + col];
  result += current_temp_matrix[accessed_cell_index - 1];  // Left neighbor

  // Apply thermal diffusivity, interval duration, and area
  // and add the current temperature
  result *= mult_constant;
  result += current_temp_matrix[accessed_cell_index];

  // Store the new value in the main matrix, with new temperatures
  plate_matrix->matrix[accessed_cell_index] = result;
}

// TODO comment
void destroy_plate_matrix(plate_matrix_t* plate_matrix) {
  free(plate_matrix->matrix);
  free(plate_matrix->auxiliary_matrix);
  free(plate_matrix);
}
