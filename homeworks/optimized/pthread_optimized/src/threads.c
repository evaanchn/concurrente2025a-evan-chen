// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "threads.h"

/**
 * @brief Calculates the last row index (exclusive) a thread should process
 *        in a row-wise data partitioning.
 *
 * This function evenly divides `evaluated_rows` among `thread_count` threads.
 * If the division leaves a remainder (i.e., `evaluated_rows % thread_count`),
 * the first few threads (with `thread_number` < remainder) get one extra row.
 *
 * In short, it represents the distribution formula for static map by blocks
 *
 * @param thread_number The index of the current thread (0-based).
 * @param evaluated_rows Total number of rows to be distributed among threads.
 * @param thread_count Total number of threads participating in the task.
 * @return The exclusive finish row index assigned to the thread.
 */
uint64_t get_finish_row(size_t thread_number, uint64_t evaluated_rows
    , size_t thread_count);

int init_shared_data(shared_data_t* shared_data, plate_t* plate
    , uint64_t thread_count) {
  // Precompute constant for temperature update calculations
  double diff_times_interval =
      plate->thermal_diffusivity * plate->interval_duration;
  double cell_area = plate->cells_dimension * plate->cells_dimension;
  double mult_constant = diff_times_interval / cell_area;

  shared_data->plate_matrix = plate->plate_matrix;
  shared_data->mult_constant = mult_constant;
  shared_data->epsilon = plate->epsilon;
  shared_data->equilibrated_plate = true;

  uint64_t evaluated_rows = plate->plate_matrix->rows - 2;
  // Thread count depends on whether there are more threads solicited
  // or more rows to evaluates
  shared_data->thread_count = evaluated_rows > thread_count ?
      thread_count : evaluated_rows;
  shared_data->k_states = 0;

  int error = EXIT_SUCCESS;
  error = pthread_mutex_init(&shared_data->can_access_equilibrated
      , /*attr*/ NULL);
  if (error != EXIT_SUCCESS) return error;

  error = pthread_barrier_init(&shared_data->can_continue1, /*attr*/ NULL
      , shared_data->thread_count);
  error = pthread_barrier_init(&shared_data->can_continue2, /*attr*/ NULL
    , shared_data->thread_count);
  if (error != EXIT_SUCCESS) {
    pthread_mutex_destroy(&shared_data->can_access_equilibrated);
    return error;
  }

  return error;
}

private_data_t* init_private_data(void* data) {
  shared_data_t* shared_data = (shared_data_t*) data;
  // Excluding first and last row, the rest of the rows are evaluated.
  uint64_t evaluated_rows = shared_data->plate_matrix->rows - 2;
  private_data_t* private_data = (private_data_t*)
      calloc(shared_data->thread_count, sizeof(private_data_t));
  if (private_data) {
    uint64_t prev_finish_row = 1;  // Initialize in 1, as row 0 is not evaluated
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count;
        ++thread_number) {
      // Starting row will be last one's finish row
      private_data[thread_number].starting_row = prev_finish_row;
      private_data[thread_number].finish_row = get_finish_row(thread_number + 1
          , evaluated_rows, shared_data->thread_count) + 1;
      prev_finish_row = private_data[thread_number].finish_row;
      private_data[thread_number].equilibrated = true;
      private_data[thread_number].shared_data = data;
    }
  }
  return private_data;
}

uint64_t get_finish_row(size_t thread_number, uint64_t evaluated_rows
  , size_t thread_count) {
  // Add the residue if thread number exceeds it
  size_t added = thread_number < evaluated_rows % thread_count ? thread_number
      : evaluated_rows % thread_count;
  return thread_number * (evaluated_rows / thread_count) + added;
}

// MODIFIED FROM IN-CLASS EXAMPLE
int create_threads(void*(*routine)(void*), void* data) {
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = (shared_data_t*) private_data[0].shared_data;
  for (size_t index = 0; index < shared_data->thread_count; ++index) {
    if (pthread_create(&private_data[index].thread_id, NULL, routine
      , &private_data[index]) != 0) {
      fprintf(stderr, "Error: could not create thread %zu\n", index);
      join_threads(index, private_data);
      return ERR_CREATE_THREAD;
    }
  }
  return EXIT_SUCCESS;
}

// MODIFIED FROM IN-CLASS EXAMPLE
int join_threads(const size_t count, private_data_t* private_data) {
  shared_data_t* shared_data = private_data[0].shared_data;

  int error_count = 0;
  for (size_t index = 0; index < count; ++index) {
    const int error = pthread_join(private_data[index].thread_id, NULL);
    if (error) {
      pthread_mutex_destroy(&shared_data->can_access_equilibrated);
      pthread_barrier_destroy(&shared_data->can_continue1);
      pthread_barrier_destroy(&shared_data->can_continue2);

      fprintf(stderr, "Error: could not join thread %zu\n", index);
      ++error_count;
    }
  }

  pthread_mutex_destroy(&shared_data->can_access_equilibrated);
  pthread_barrier_destroy(&shared_data->can_continue1);
  pthread_barrier_destroy(&shared_data->can_continue2);

  return error_count;
}
