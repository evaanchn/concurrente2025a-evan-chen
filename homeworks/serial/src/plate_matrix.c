// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate_matrix.h"
#include <assert.h>
#include <stdlib.h>

plate_matrix_t* init_plate_matrix(uint64_t rows, uint64_t cols) {
    plate_matrix_t* plate_matrix = (plate_matrix_t*) 
        malloc(sizeof(plate_matrix_t));
    if (!plate_matrix) return NULL; // Handle allocation failure

    plate_matrix->rows = rows;
    plate_matrix->cols = cols;
    plate_matrix->matrix = create_double_matrix(rows, cols);
    
    if (!plate_matrix->matrix) { 
        free(plate_matrix); 
        return NULL; 
    }

    plate_matrix->auxiliary_matrix = create_double_matrix(rows, cols);
    if (!plate_matrix->auxiliary_matrix) { 
        free(plate_matrix->matrix); // Free matrix before returning
        free(plate_matrix); 
        return NULL; 
    }

    return plate_matrix;
}


void init_auxiliary(plate_matrix_t* plate_matrix) {
  double** initial_matrix = plate_matrix->matrix;
  
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    plate_matrix->auxiliary_matrix[0][col] = initial_matrix[0][col];
  }

  uint64_t last_row = plate_matrix -> rows - 1;
  for (uint64_t col = 0; col < plate_matrix->cols; ++col) {
    plate_matrix->auxiliary_matrix[last_row][col] = 
        initial_matrix[last_row][col];
  }

  for (uint64_t row = 1; row < last_row; ++row) {
    plate_matrix->auxiliary_matrix[row][0] = initial_matrix[row][0];
  }

  uint64_t last_col = plate_matrix -> cols - 1;
  for (uint64_t row = 1; row < last_row; ++row) {
    plate_matrix->auxiliary_matrix[row][last_col] = initial_matrix[row][last_col];
  }
}

void set_auxiliary(plate_matrix_t* plate_matrix) {
    double** old_temperatures = plate_matrix->matrix;
    plate_matrix->matrix = plate_matrix->auxiliary_matrix;
    plate_matrix->auxiliary_matrix = old_temperatures;
}

void update_cell(plate_matrix_t* plate_matrix, uint64_t row, uint64_t col,
    double mult_constant) {
  // Aux holds current temp
  double** current_temp_matrix = plate_matrix->auxiliary_matrix;

  // Net energy released
  double result = -4 * current_temp_matrix[row][col];

  // Add top neighbor energy
  result += current_temp_matrix[row-1][col];

  // Add right neighbor energy
  result += current_temp_matrix[row][col+1];

  // Add bottom neighbor energy
  result += current_temp_matrix[row+1][col];

  // Add left neighbor energy
  result += current_temp_matrix[row][col-1];

  // result := result * thermal_diffusivity * interval_duration / cell_area
  result *= mult_constant;

  // result := result + matrix[row][col]
  result += current_temp_matrix[row][col];

  // Update new temperature in official matrix
  plate_matrix->matrix[row][col] = result;
}

// CODE FROM TEAM_SHOT_PUT IN PTHREADS/TEAM_SHOT_PUT/SRC
double** create_double_matrix(const size_t rows, const size_t cols) {
  double** matrix = (double**) calloc(rows, sizeof(double*));
  if (matrix) {
    for (size_t row = 0; row < rows; ++row) {
      if ((matrix[row] = (double*) calloc(cols, sizeof(double))) == NULL) {
        destroy_double_matrix(matrix, rows);
        return NULL;
      }
    }
  }
  return matrix;
}

void destroy_matrices(plate_matrix_t* plate_matrix) {
  uint64_t rows = plate_matrix->rows;

  // Destroy main matrix
  destroy_double_matrix(plate_matrix->matrix, rows);

  // Destroy auxiliary matrix
  destroy_double_matrix(plate_matrix->auxiliary_matrix, rows);
}

// CODE FROM TEAM_SHOT_PUT IN PTHREADS/TEAM_SHOT_PUT/SRC
void destroy_double_matrix(double** matrix, const uint64_t rows) {
  if (matrix) {
    for (size_t row = 0; row < rows; ++row) {
      free(matrix[row]);
    }
    free(matrix);
  }
}
