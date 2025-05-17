// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "threads.h"

int init_shared_data(shared_data_t* shared_data, uint64_t thread_count) {
  shared_data->thread_count = thread_count;
  shared_data->stop_condition = 0;
  if (queue_init(&shared_data->rows_queue) != EXIT_SUCCESS) {
    fprintf(stderr, "Error: could not initialize queue");
    return ERR_QUEUE_INIT;
  }

  // Initialize in 0 since main thread must signal to allow consumption
  if (sem_init(&shared_data->can_get_working_row, /*pshared*/ 0
      , /*value*/ 0) != EXIT_SUCCESS) {
    fprintf(stderr, "Error: could not initialize queue semaphore");
    return ERR_CONSUME_SEM_INIT;
  }

  if (pthread_mutex_init(&shared_data->can_access_equilibrated, /*attr*/ NULL)
      != EXIT_SUCCESS) {
    fprintf(stderr, "Error: could not initialize mutex");
    return ERR_EQULIBRATED_PLATE_MUTEX_INIT;
  }

  // Initialize in 0 given it is a signal system
  if (sem_init(&shared_data->state_done, /*pshared*/ 0
      , /*value*/ 0) != EXIT_SUCCESS) {
    fprintf(stderr, "Error: could not initialize states semaphore");
    return ERR_STATE_DONE_SEM_INIT;
  }
  return EXIT_SUCCESS;
}

private_data_t* init_private_data(const size_t count, void* data) {
  shared_data_t* shared_data = (shared_data_t*) data;
  
  private_data_t* private_data = (private_data_t*)
      calloc(count, sizeof(private_data_t));
  if (private_data) {
    for (size_t index = 0; index < shared_data->thread_count; ++index) {
      private_data[index].shared_data = data;
    }
  }
  return private_data;
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
  int error_count = 0;
  for (size_t index = 0; index < count; ++index) {
    const int error = pthread_join(private_data[index].thread_id, NULL);
    if (error) {
      fprintf(stderr, "Error: could not join thread %zu\n", index);
      ++error_count;
    }
  }
  return error_count;
}

void destroy_shared_data(shared_data_t* shared_data) {
  queue_destroy(&shared_data->rows_queue);
  sem_destroy(&shared_data->can_get_working_row);
  pthread_mutex_destroy(&shared_data->can_access_equilibrated);
  sem_destroy(&shared_data->state_done);
}
