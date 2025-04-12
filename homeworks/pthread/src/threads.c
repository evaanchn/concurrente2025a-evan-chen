// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "threads.h"

/**
 * @brief Calculates the interval of rows each thread should process.
 *
 * Determines how many rows each thread should evaluate based on the
 * number of available rows and the number of threads.
 *
 * @param shared_data A pointer to the shared_data_t structure.
 * @return The number of rows to be evaluated per thread.
 */
uint64_t get_intervals(shared_data_t* shared_data);

private_data_t* init_private_data(const size_t count, void* data) {
  shared_data_t* shared_data = (shared_data_t*) data;
  // last eval row will be rows - 2, the row before last row
  uint64_t last_eval_row = shared_data->plate_matrix->rows - 2;
  // Thread count depends on whether there are more threads solicited
  // or more rows to evaluate: if the last evaluating row is greater,
  // then we can only set thread_count as the ones solicited
  // if there are more threads than rows, then we limit the count to
  // rows needed
  shared_data->thread_count = last_eval_row > count ? count : last_eval_row;

  private_data_t* private_data = (private_data_t*)
      calloc(shared_data->thread_count, sizeof(private_data_t));
  if (private_data) {
    uint64_t interval_to_end = get_intervals(shared_data) - 1;
    uint64_t row_pointer = 0;  // Starts at first row

    for (size_t index = 0; index < shared_data->thread_count; ++index) {
      private_data[index].starting_row = ++row_pointer;  // Inc prev ending
      row_pointer += interval_to_end;
      private_data[index].ending_row = (index == shared_data->thread_count - 1)
          ? last_eval_row  // If it's the last one, set it to the last eval row
          : row_pointer;
      private_data[index].equilibrated = true;  // Assume reached equilibrium
      private_data[index].shared_data = data;
    }
  }
  return private_data;
}

uint64_t get_intervals(shared_data_t* shared_data) {
  // Eliminate first and last row from calculations
  uint64_t evaluating_rows = shared_data->plate_matrix->rows - 2;
  uint64_t intervals = 1;  // Assume intervals are 1 first
  // If thread count solicited is greater than evaluating rows,
  // intervals of 1 would stay valid, if not, must change
  if (shared_data->thread_count < evaluating_rows) {
    intervals = evaluating_rows / shared_data->thread_count;
  }
  return intervals;
}

// MODIFIED FROM IN-CLASS EXAMPLE
int create_threads(void*(*routine)(void*), void* data) {
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = (shared_data_t*) private_data[0].shared_data;
  for (size_t index = 0; index < shared_data->thread_count; ++index) {
    if (pthread_create(&private_data[index].thread_id, NULL, routine
      , &private_data[index]) != 0) {
      fprintf(stderr, "Error: could not create thread %zu\n", index);
      bool temp = true;
      join_threads(index, private_data, &temp);
      return ERR_CREATE_THREAD;
    }
  }
  return EXIT_SUCCESS;
}

// MODIFIED FROM IN-CLASS EXAMPLE
int join_threads(const size_t count, private_data_t* private_data
  , bool* reached_equilibrium) {
  int error_count = 0;
  for (size_t index = 0; index < count; ++index) {
    const int error = pthread_join(private_data[index].thread_id, NULL);
    if (error) {
      fprintf(stderr, "Error: could not join thread %zu\n", index);
      ++error_count;
    }
    *reached_equilibrium = *reached_equilibrium
        && private_data[index].equilibrated;
    private_data[index].equilibrated = true;
  }
  return error_count;
}

