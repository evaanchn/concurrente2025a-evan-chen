// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate_matrix.h"
#include <stdlib.h>
#include <stdio.h>

plate_matrix_t* init_plate_matrix(uint64_t rows, uint64_t cols) {
  // Allocate memory for the plate_matrix_t structure
  plate_matrix_t* plate_matrix =
      (plate_matrix_t*)malloc(sizeof(plate_matrix_t));

  if (!plate_matrix) return NULL;  // Handle allocation failure

  // Assign matrix dimensions
  plate_matrix->rows = rows;
  plate_matrix->cols = cols;

  // Allocate the main matrix
  plate_matrix->matrix = create_double_matrix(rows, cols);
  if (!plate_matrix->matrix) {
    free(plate_matrix);
    return NULL;
  }

  // Allocate the auxiliary matrix
  plate_matrix->auxiliary_matrix = create_double_matrix(rows, cols);
  if (!plate_matrix->auxiliary_matrix) {
    free(plate_matrix->matrix);  // Free main matrix before returning
    free(plate_matrix);
    return NULL;
  }

  return plate_matrix;
}

void init_auxiliary(plate_matrix_t* plate_matrix) {
  double** initial_matrix = plate_matrix->matrix;

  // Copy first row to auxiliary matrix
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    plate_matrix->auxiliary_matrix[0][col] = initial_matrix[0][col];
  }

  // Copy last row to auxiliary matrix
  uint64_t last_row = plate_matrix->rows - 1;
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    plate_matrix->auxiliary_matrix[last_row][col] =
        initial_matrix[last_row][col];
  }

  // Copy first column to auxiliary matrix
  for (uint64_t row = 1; row < last_row; ++row) {
    plate_matrix->auxiliary_matrix[row][0] = initial_matrix[row][0];
  }

  // Copy last column to auxiliary matrix
  uint64_t last_col = plate_matrix->cols - 1;
  for (uint64_t row = 1; row < last_row; ++row) {
    plate_matrix->auxiliary_matrix[row][last_col] =
        initial_matrix[row][last_col];
  }
}

void set_auxiliary(plate_matrix_t* plate_matrix) {
  // Temperatures at current state are in matrix, but we want them to be
  // stored in auxiliary to have space in the main matrix for new temperatures
  double** current_temperatures = plate_matrix->matrix;

  // Set matrix pointer to auxiliary's, which holds k-1 state temperatures
  // These will be overwritten
  plate_matrix->matrix = plate_matrix->auxiliary_matrix;

  // Auxiliary matrix now has the current temperatures
  // to properly make calculations
  plate_matrix->auxiliary_matrix = current_temperatures;
}

void update_cell(plate_matrix_t* plate_matrix, uint64_t row,
      uint64_t col, double mult_constant) {
  double** current_temp_matrix = plate_matrix->auxiliary_matrix;

  // Compute net energy change using the heat diffusion equation
  double result = -4 * current_temp_matrix[row][col];
  result += current_temp_matrix[row - 1][col];  // Top neighbor
  result += current_temp_matrix[row][col + 1];  // Right neighbor
  result += current_temp_matrix[row + 1][col];  // Bottom neighbor
  result += current_temp_matrix[row][col - 1];  // Left neighbor

  // Apply thermal diffusivity, interval duration, and area
  // and add the current temperature
  result *= mult_constant;
  result += current_temp_matrix[row][col];

  // Store the new value in the main matrix, with new temperatures
  plate_matrix->matrix[row][col] = result;
}

// CODE FROM TEAM_SHOT_PUT
double** create_double_matrix(const size_t rows, const size_t cols) {
  double** matrix = (double**)calloc(rows, sizeof(double*));
  if (matrix) {
    // Initialize rows
    for (size_t row = 0; row < rows; ++row) {
      // Make sure row was allocated successfully with if
      if ((matrix[row] = (double*)calloc(cols, sizeof(double))) == NULL) {
          destroy_double_matrix(matrix, rows);
          return NULL;
      }
    }
  }
  return matrix;
}

void destroy_matrices(plate_matrix_t* plate_matrix) {
    uint64_t rows = plate_matrix->rows;

    // Calls double matrix destruction subroutines
    destroy_double_matrix(plate_matrix->matrix, rows);
    destroy_double_matrix(plate_matrix->auxiliary_matrix, rows);
}

// CODE FROM TEAM_SHOT_PUT
void destroy_double_matrix(double** matrix, const uint64_t rows) {
  if (matrix) {
    // Free by row
    for (size_t row = 0; row < rows; ++row) {
        free(matrix[row]);
    }
    free(matrix);
  }
}
