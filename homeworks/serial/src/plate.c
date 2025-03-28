// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate.h"

int set_plate_matrix(plate_t* plate, char* source_directory) {
  char* plate_file_path = build_file_path(source_directory, plate->file_name);

  if (!plate_file_path) {
    perror("Error: Plate file path could not be built");
    return EXIT_FAILURE;
  }

  FILE* plate_file = fopen(plate_file_path, "rb");
  free(plate_file_path);

  if (!plate_file) {
    printf("Error: Plate file %s could not be opened", plate->file_name);
    return 31;
  }

  uint64_t rows, cols;
  if (fread(&rows, sizeof(uint64_t), 1, plate_file) != 1 ||
      fread(&cols, sizeof(uint64_t), 1, plate_file) != 1) {
    perror("Error: Rows and cols could not be read");
    fclose(plate_file);
    return 32;
  }

  plate->plate_matrix = init_plate_matrix(rows, cols);
  double** matrix = plate->plate_matrix->matrix;

  for (size_t row = 0; row < rows; ++row) {
    // Read cols amount of doubles (a row) from plate_file to store
    fread(matrix[row], sizeof(double), cols, plate_file);
  }

  init_auxiliary(plate->plate_matrix);

  fclose(plate_file);
  return EXIT_SUCCESS;
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

  for (size_t row = 1; row < plate_matrix->rows - 1; ++row) {
    for (size_t col = 1; col < plate_matrix->cols - 1; ++col) {
      update_cell(plate_matrix, row, col, mult_constant);

      double new_temperature = plate_matrix -> matrix[row][col];
      double old_temperature = plate_matrix -> auxiliary_matrix[row][col];
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

int update_plate_file(plate_t* plate) {
  int error = EXIT_SUCCESS;

  const char* output_file_name = plate->file_name;
  // TODO (Evan Chen): Add folder path output/
  FILE* output_file = fopen(output_file_name, "wb");
  
  if (output_file) { 
    plate_matrix_t* plate_matrix = plate->plate_matrix;

    fwrite(&plate_matrix->rows, sizeof(uint64_t), 1, output_file);
    fwrite(&plate_matrix->cols, sizeof(uint64_t), 1, output_file);

    for (uint64_t row = 0; row < plate_matrix->rows; ++row) {
      fwrite(plate_matrix->matrix[row], sizeof(double),
          plate_matrix->cols, output_file);
    }
  } else {
    perror("Error: Could not open output file");
    destroy_matrices(plate->plate_matrix);
    error = EXIT_FAILURE;
  }

  fclose(output_file);
  return error;
}
