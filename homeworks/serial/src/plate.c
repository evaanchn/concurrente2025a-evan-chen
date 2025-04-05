// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate.h"
// TODO (e): Fuse plate and plate_matrix_t
int set_plate_matrix(plate_t* plate, char* source_directory) {
  // Concatenate plate file name with same directory specified for job
  char* plate_file_path = build_file_path(source_directory, plate->file_name);

  if (!plate_file_path) return EXIT_FAILURE;

  // Open file to read from
  FILE* plate_file = fopen(plate_file_path, "rb");
  free(plate_file_path);

  if (!plate_file) {
    printf("Error: Plate file %s could not be opened", plate->file_name);
    return OPEN_PLATE_FILE_FAIL;
  }

  // Read number of rows and number of columns (first 16 bytes)
  uint64_t rows = 0, cols = 0;

  // Handle exception if reading is unsuccessful
  if (fread(&rows, sizeof(uint64_t), 1, plate_file) != 1 ||
      fread(&cols, sizeof(uint64_t), 1, plate_file) != 1) {
    perror("Error: Rows and cols could not be read");
    fclose(plate_file);
    return ROWS_COLS_READING_FAIL;
  }

  // Set up plate's plate_matrix (allocate space for matrices inside,
  // store rows and cols)
  plate->plate_matrix = init_plate_matrix(rows, cols);
  double** matrix = plate->plate_matrix->matrix;

  for (size_t row = 0; row < rows; ++row) {
    // Read cols amount of doubles (a row) from plate_file to store
    fread(matrix[row], sizeof(double), cols, plate_file);
  }

  // Copy matrix's borders to auxiliary, to prepare for matrix switches
  init_auxiliary(plate->plate_matrix);

  fclose(plate_file);
  return EXIT_SUCCESS;
}



bool update_plate(plate_t* plate) {
  // Get the plate matrix from the plate structure
  plate_matrix_t* plate_matrix = plate->plate_matrix;

  // Prepare the auxiliary matrix for calculations
  // (switch so it points to current temps)
  set_auxiliary(plate_matrix);

  // Assume equilibrium unless a significant temperature change occurs
  bool reached_equilibrium = true;
  double biggest_change = 0;

  // Precompute constant for temperature update calculations
  double diff_times_interval =
      plate->thermal_diffusivity * plate->interval_duration;
  uint64_t cell_area = plate->cells_dimension * plate->cells_dimension;
  double mult_constant = diff_times_interval / cell_area;

  // Iterate over all interior cells (excluding boundary cells)
  for (size_t row = 1; row < plate_matrix->rows - 1; ++row) {
    for (size_t col = 1; col < plate_matrix->cols - 1; ++col) {
      // Update the cell temperature based on surrounding cells
      update_cell(plate_matrix, row, col, mult_constant);

      // Get the new and old temperatures for comparison
      double new_temperature = plate_matrix->matrix[row][col];
      double old_temperature = plate_matrix->auxiliary_matrix[row][col];

      // Compute absolute difference
      double difference = fabs(new_temperature - old_temperature);

      // Track the maximum temperature change in this update step
      if (difference > biggest_change) {
        biggest_change = difference;
      }
    }
  }

  // Determine if the plate has reached thermal equilibrium
  if (biggest_change > plate->epsilon) {
    reached_equilibrium = false;
  }

  return reached_equilibrium;
}



int update_plate_file(plate_t* plate, char* source_directory) {
  int error = EXIT_SUCCESS;

  // Generate the updated file name based on the plate's state
  char* updated_file_name = set_plate_file_name(plate);
  char* output_file_name = build_file_path(source_directory, updated_file_name);

  // Check if file path was successfully created
  if (!output_file_name) {
    free(updated_file_name);
    return EXIT_FAILURE;
  }

  // Open the file for writing in binary mode
  FILE* output_file = fopen(output_file_name, "wb");

  // Free memory of file names first, since it won't be used anymore
  free(updated_file_name);
  free(output_file_name);

  if (output_file) {
    // Retrieve the plate matrix
    plate_matrix_t* plate_matrix = plate->plate_matrix;

    // Write matrix dimensions to the file (first 16 bytes)
    fwrite(&plate_matrix->rows, sizeof(uint64_t), 1, output_file);
    fwrite(&plate_matrix->cols, sizeof(uint64_t), 1, output_file);

    // Write the matrix data to the file row by row
    for (uint64_t row = 0; row < plate_matrix->rows; ++row) {
      fwrite(plate_matrix->matrix[row], sizeof(double),
          plate_matrix->cols, output_file);
    }
  } else {
    // Handle file opening failure
    perror("Error: Could not open output file");
    destroy_plate_matrix(plate->plate_matrix);
    error = OPEN_OUTPUT_FILE_FAIL;
  }

  // Close the file before returning
  fclose(output_file);
  return error;
}



char* set_plate_file_name(plate_t* plate) {
  // Locate the last occurrence of '.' to find the file extension
  const char *last_dot = strrchr(plate->file_name, '.');

  if (!last_dot) {
    perror("Error: no extension specified for plate file\n");
    return NULL;
  }

  // Create a suffix containing the plate's state count and new extension
  char suffix[25];  // Enough for 20 digits, .bin, and null terminator
  snprintf(suffix, sizeof(suffix), "%lu.bin", plate->k_states);

  // Determine the length of the original name (excluding the extension)
  const size_t name_length = last_dot - plate->file_name;

  // Allocate memory for the new filename: name + '-' + suffix + null terminator
  const size_t new_size = name_length + strlen(suffix) + 2;
  char *new_filename = (char*)calloc(1, new_size);

  if (!new_filename) {
    perror("Error: Memory allocation failed for platefile name\n");
    return NULL;
  }

  // Construct the new filename safely
  snprintf(new_filename, new_size, "%.*s-%s",
      (int)name_length, plate->file_name, suffix);

  return new_filename;
}
