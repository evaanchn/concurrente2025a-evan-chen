// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "plate.h"
#include "threads.h"

int set_plate_matrix(plate_t* plate, char* source_directory) {
  // Concatenate plate file name with same directory specified for job
  char* plate_file_path = build_file_path(source_directory, plate->file_name);

  if (!plate_file_path) return EXIT_FAILURE;

  // Open file to read from
  FILE* plate_file = fopen(plate_file_path, "rb");
  free(plate_file_path);

  if (!plate_file) {
    printf("Error: Plate file %s could not be opened", plate->file_name);
    return ERR_OPEN_PLATE_FILE;
  }

  // Read number of rows and number of columns (first 16 bytes)
  uint64_t rows = 0, cols = 0;

  // Handle exception if reading is unsuccessful
  if (fread(&rows, sizeof(uint64_t), 1, plate_file) != 1 ||
      fread(&cols, sizeof(uint64_t), 1, plate_file) != 1) {
    perror("Error: Rows and cols could not be read");
    fclose(plate_file);
    return ERR_ROWS_COLS;
  }

  // Set up plate's plate_matrix (allocate space for matrices inside,
  // store rows and cols)
  plate->plate_matrix = init_plate_matrix(rows, cols);
  double* row_start = plate->plate_matrix->matrix;

  for (size_t row = 0; row < rows; ++row) {
    // Read cols amount of doubles (a row) from plate_file to store
    fread(row_start, sizeof(double), cols, plate_file);
    row_start += plate->plate_matrix->cols;
  }

  // Copy matrix's borders to auxiliary, to prepare for matrix switches
  init_auxiliary(plate->plate_matrix);

  fclose(plate_file);
  return EXIT_SUCCESS;
}


void* equilibrate_rows(void* data) {
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  plate_matrix_t* plate_matrix = shared_data->plate_matrix;
  uint64_t starting_row = private_data->starting_row;
  uint64_t ending_row = private_data->finish_row;
  // Only work designated rows
  for (uint64_t row = starting_row; row <= ending_row; ++row) {
    for (uint64_t col = 1; col < plate_matrix->cols - 1; ++col) {
      // Update the cell temperature based on surrounding cells
      update_cell(plate_matrix, row, col, shared_data->mult_constant);
      uint64_t accessed_index = row * plate_matrix->cols + col;
      // Get the new and old temperatures for comparison
      double new_temperature = plate_matrix->matrix[accessed_index];
      double old_temperature = plate_matrix->auxiliary_matrix[accessed_index];

      // Compute absolute difference
      double difference = fabs(new_temperature - old_temperature);
      // Track the maximum temperature change in this update step
      if (difference > shared_data->epsilon) {
        private_data->equilibrated = false;
      }
    }
  }
  return NULL;
}
void* equilibrate_plate_concurrent(void* data) {
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  plate_matrix_t* plate_matrix = shared_data->plate_matrix;

  while (true) {
    // Reset local flag for this round
    private_data->equilibrated = true;
    // Update rows; this may set equilibrated = false
    equilibrate_rows(data);

    // Combine result into shared flag
    pthread_mutex_lock(&shared_data->can_access_equilibrated);
      shared_data->equilibrated_plate &= private_data->equilibrated;
    pthread_mutex_unlock(&shared_data->can_access_equilibrated);

    // First barrier: sync all threads after work
    // Second barrier: ensure all threads see updated matrix and equilibrium result
    printf("Thread %lu reached barrier 1\n", private_data->thread_id);
    int barrier_result = pthread_barrier_wait(&shared_data->can_continue1);
    printf("Thread %lu passed barrier 1\n", private_data->thread_id);
    if (barrier_result == PTHREAD_BARRIER_SERIAL_THREAD) {
      ++shared_data->k_states;
      set_auxiliary(plate_matrix);
      printf("Equilibrated state %" PRIu64 "\n", shared_data->k_states);
    }

    // Second barrier: ensure all threads see updated matrix and equilibrium result
    printf("Thread %lu reached barrier 2\n", private_data->thread_id);
    pthread_barrier_wait(&shared_data->can_continue2);
    printf("Thread %lu passed barrier 2\n", private_data->thread_id);


    // Check if we’re done
    pthread_mutex_lock(&shared_data->can_access_equilibrated);
      bool done = shared_data->equilibrated_plate;
    pthread_mutex_unlock(&shared_data->can_access_equilibrated);

    if (done) {
      printf("Thread %lu left\n", private_data->thread_id);
      break;
    }
    else printf("Thread %lu moved on\n", private_data->thread_id);

    if (pthread_barrier_wait(&shared_data->can_continue1)
        == PTHREAD_BARRIER_SERIAL_THREAD) {
      shared_data->equilibrated_plate = true;
    }
  }

  return NULL;
}



int update_plate_file(plate_t* plate, char* source_directory) {
  int error = EXIT_SUCCESS;

  // Generate the updated file name based on the plate's state
  char* updated_file_name = set_plate_file_name(plate);
  if (!updated_file_name) return ERR_UPDATE_OUTPUT_FILE_NAME;

  char* output_file_name = build_file_path(source_directory, updated_file_name);

  // Check if file path was successfully created
  if (!output_file_name) {
    perror("Error: Could not build output file name");
    free(updated_file_name);
    return ERR_BUILD_OUTPUT_FILE_NAME;
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
    double* row_start = plate_matrix->matrix;
    // Write the matrix data to the file row by row
    for (uint64_t row = 0; row < plate_matrix->rows; ++row) {
      fwrite(row_start, sizeof(double),
          plate_matrix->cols, output_file);
      row_start += plate_matrix->cols;
    }
  } else {
    // Handle file opening failure
    perror("Error: Could not open output file");
    destroy_plate_matrix(plate->plate_matrix);
    error = ERR_OPEN_OUTPUT_FILE;
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
