// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "threads.h"

uint64_t get_finish_row(size_t thread_number, uint64_t evaluated_rows
    , size_t thread_count);


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
    uint64_t prev_finish_row = 1;  // Initialize in 1, as row 0 is not evaluated
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count;
        ++thread_number) {
      // Starting row will be last one's finish row
      private_data[thread_number].starting_row = prev_finish_row;
      private_data[thread_number].ending_row = get_finish_row(thread_number + 1
          , last_eval_row, shared_data->thread_count) + 1;
      prev_finish_row = private_data[thread_number].ending_row;
      private_data[thread_number].equilibrated = true;
      private_data[thread_number].shared_data = data;
      printf("Start: %" PRIu64 " End: %" PRIu64 "\n", private_data[thread_number].starting_row, private_data[thread_number].ending_row);

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

