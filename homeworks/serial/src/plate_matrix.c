// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate_matrix.h"

plate_matrix_t* init_plate_matrix(uint64_t rows, uint64_t cols) {
  plate_matrix_t* plate_matrix =
      (plate_matrix_t*) calloc(1, sizeof(plate_matrix_t));
  if (plate_matrix) {
    plate_matrix->rows = rows;
    plate_matrix->cols = cols;
    plate_matrix->matrix = create_double_matrix(rows, cols);
    plate_matrix->auxiliary_matrix = create_double_matrix(rows, cols);

    // TODO (evan.chen): Adapt asserts to ifs later
    assert(plate_matrix->matrix);
    assert(plate_matrix->auxiliary_matrix);

    return plate_matrix;
  } else {
      return NULL;
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
double** create_double_matrix(const uint64_t rows, const uint64_t cols) {
    double** matrix = calloc(rows, sizeof(double*));
  if (matrix) {
    for (size_t row = 0; row < rows; ++row) {
      if ((matrix[row] = calloc(cols, sizeof(double))) == NULL) {
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
