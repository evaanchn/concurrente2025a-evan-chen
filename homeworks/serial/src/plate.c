// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate.h"

void set_plate_matrix(plate_t* plate) {
  const char* file_name = plate->file_name;
  FILE* plate_file = fopen(file_name, "rb");
  uint64_t rows, cols;
  fread(&rows, sizeof(uint64_t), 1, plate_file);
  fread(&cols, sizeof(uint64_t), 1, plate_file);

  plate->plate_matrix = init_plate_matrix(rows, cols);

  for (size_t row = 0; row < rows; ++row) {
    // Read cols amount of doubles (a row) from plate_file to store
    fread(&plate->plate_matrix[row], sizeof(double), cols, plate_file);
  }

  fclose(plate_file);
}

bool update_plate(plate_t* plate) {
  plate_matrix_t* plate_matrix = plate->plate_matrix;
  set_auxiliary(plate_matrix);
  bool reached_equilibrium = true;
  double biggest_change = 0;

  double diff_times_interval = plate->thermal_diffusivity
      * plate->interval_duration;
  uint64_t cell_area = plate->cells_dimension * plate->cells_dimension;
  double mult_constant = diff_times_interval / cell_area;

  for (size_t row = 1; row < plate_matrix->rows; ++row) {
    for (size_t col = 1; col < plate_matrix->cols; ++col) {
      update_cell(plate_matrix, row, col, mult_constant);

      double new_temperature = plate_matrix -> matrix[row][col];
      double old_temperature = plate_matrix -> auxiliary_matrix[row][col];
      // TODO (Evan Chen): Verify fabs() works for abs
      double difference = fabs(new_temperature - old_temperature);  // ABS

      if (difference > biggest_change) {
        biggest_change = difference;
      }
    }
  }

  if (biggest_change > plate->epsilon) {
    reached_equilibrium = false;
  }

  return reached_equilibrium;
}

void update_plate_file(plate_t* plate) {
  const char* output_file_name = plate->file_name;
  // TODO (Evan Chen): Add folder path output/
  FILE* output_file = fopen(output_file_name, "wb");
  plate_matrix_t* plate_matrix = plate->plate_matrix;

  fwrite(plate_matrix->rows, sizeof(uint64_t), 1, output_file);
  fwrite(plate_matrix->cols, sizeof(uint64_t), 1, output_file);

  for (uint64_t row = 0; row < plate_matrix->rows; ++row) {
    // TODO (Evan Chen): Figure out if this works
    fwrite(plate_matrix->matrix[row], sizeof(double),
        plate_matrix->cols, output_file);
  }

  fclose(output_file);
}
