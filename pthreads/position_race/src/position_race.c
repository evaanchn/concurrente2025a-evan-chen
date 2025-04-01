// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen

#define _POSIX_C_SOURCE 199309L // To enable CLOCK_MONOTONIC

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// thread_shared_data_t
typedef struct shared_data {
  // Current position - 1 that can be taken
  uint64_t position;

  // Mutual exclusion to control who gets to increment position and announce
  pthread_mutex_t can_access_position;
  uint64_t thread_count;
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // rank
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief Increments the position and announces in standard output
 * @param data, private_data of the thread
 */
void* race(void* data);

/**
 * @brief Creates and later joins threads used to call race(), with their
 *        private data as argument.
 * @param shared_data: Amount of threads to create, but now in a register that
 *        all threads can access
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(shared_data_t* shared_data);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;

  // Usual thread count initialization
  // create thread_count as result of converting argv[1] to integer
  // thread_count := integer(argv[1])
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  // Initialize shared_data
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) {
    shared_data->position = 0;
    
    // INITIALIZATION OF MUTEX: pass address of mutex in shared_data
    error = pthread_mutex_init(&shared_data->can_access_position, /*attr*/NULL);

    if (error == EXIT_SUCCESS) {
      shared_data->thread_count = thread_count;

      // To record start time
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      error = create_threads(shared_data);

      // Record end time
      clock_gettime(CLOCK_MONOTONIC, &finish_time);

      // Calculate elapsed time in seconds
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      // Report execution time
      printf("Execution time: %.9lfs\n", elapsed_time);

      // DESTRUCTION OF MUTEX: Do before freeing shared data
      pthread_mutex_destroy(&shared_data->can_access_position);
      free(shared_data);
    } else {
      fprintf(stderr, "Error: could not init mutex\n");
      return 13;
    }
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    return 12;
  }
  return error;
}  // end procedure


int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  pthread_t* threads = (pthread_t*)
    malloc(shared_data->thread_count * sizeof(pthread_t));

  // Allocate memory for private data
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    // Create threads based on amount specified 
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // Initialize private data values
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].shared_data = shared_data;
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, race
        , /*arg*/ &private_data[thread_number]);
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    // Loop to join the threads
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 22;
  }

  return error;
}

// procedure race:
void* race(void* data) {
  assert(data);

  // Obtain private and shared data
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  // LOCKING OF MUTEX: indicates a thread is in the "narrow bridge"
  /// lock(can_access_position)
  pthread_mutex_lock(&shared_data->can_access_position);
// Start of critical region
{
  // race condition/data race:
  // modificación concurrente de memoria compartida
  // In this case, position is the shared variable that will be modified by
  // multiple threads at the same time, meaning we have to protect it

  // position := position + 1
  ++shared_data->position;

  // my_position := position
  uint64_t my_position = shared_data->position;

  // We include the print inside this because 1) we want to report in order
  // and 2) it is not that big of an operation
  // print "Hello from secondary thread"
  printf("Thread %" PRIu64 "/%" PRIu64 ": I arrived at position %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count, my_position);
}
// Critical region ends

  // UNLOCKING OF MUTEX: Forgetting to do this may result in a "sleeping beauty"
  // procedure, where threads are waiting to be assigned in queue and no one 
  // is there to wake them. CPU is not used by the program in this case.
  // unlock(can_access_position)
  pthread_mutex_unlock(&shared_data->can_access_position);
  return NULL;
}  // end procedure