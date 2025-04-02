// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

#define _POSIX_C_SOURCE 199309L // To enable CLOCK_MONOTONIC

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_GREET_LEN 256  // Limits amount of characters a greeting can have

// thread_shared_data_t
typedef struct shared_data {
  // An array of greeting messages,
  // each thread with its own designated storage space
  char** greets;
  uint64_t thread_count;
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // rank
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief Procedure where a thread can store in its index on the greeting array,
 *        the greeting in the thread_number of thread_count format
 * @param data, private data of thread with thread number and reference to
 *        shared memory, with the greeetings array and total thread count
 */
void* greet(void* data);

/**
 * @brief Creates and later joins threads used to call greet, sending necessary
 *        arguments to each thread.
 * @param shared_data: Amount of threads to create, but now in a register that
 *        all threads can access
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(shared_data_t* shared_data);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  // Set thread count, default if no second argument, or else the specified one
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

  // Allocate memory for shared data
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) {
    // Allocate memory for greetings array
    shared_data->greets = (char**) calloc(thread_count, sizeof(char*));
    shared_data->thread_count = thread_count;  // Set thread count in shr_data

    // If allocation successful
    if (shared_data->greets) {
      // Record start time
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      // Excecute concurrent code
      error = create_threads(shared_data);

      // Record end time
      clock_gettime(CLOCK_MONOTONIC, &finish_time);

      // Set elapsed time
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      // REport elapsed time
      printf("Execution time: %.9lfs\n", elapsed_time);

      free(shared_data->greets);
    } else {
      fprintf(stderr, "Error: could not allocate greets\n");
      error = 13;
    }
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = 12;
  }
  return error;
}  // end procedure


int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  // Allocate mem for threads and private data
  pthread_t* threads = (pthread_t*)
    malloc(shared_data->thread_count * sizeof(pthread_t));
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // Set up current thread's buffer by allocating memory for chars
      shared_data->greets[thread_number] = (char*)
        malloc(MAX_GREET_LEN * sizeof(char));
      if (shared_data->greets[thread_number]) {
        // Set up private memory's attributes
        // *shared_data->greets[thread_number] = '\0';
        // This means trash values the string may have after first position
        // are ignored, effectively setting the buffer as empty
        shared_data->greets[thread_number][0] = '\0';
        private_data[thread_number].thread_number = thread_number;
        private_data[thread_number].shared_data = shared_data;
        // create_thread(greet, thread_number)
        error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
          , /*arg*/ &private_data[thread_number]);
        if (error == EXIT_SUCCESS) {
        } else {
          fprintf(stderr, "Error: could not create secondary thread\n");
          error = 21;
          break;
        }
      } else {
        fprintf(stderr, "Error: could not init semaphore\n");
        error = 22;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    // Joining each thread created
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    // Print all greetings at once from main thread
    // for thread_number := 0 to thread_count do
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // print greets[thread_number]
      printf("%s\n", shared_data->greets[thread_number]);
      free(shared_data->greets[thread_number]);  // Free the greeting
    }  // end for

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  assert(data);
  // Set up private data and shared data
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  // Report which thread is writing its greeting (shows indeterminism)
  printf("  %" PRIu64 "/%" PRIu64 ": write greeting\n"
    , private_data->thread_number, shared_data->thread_count);

  // Write greeting in designated area (meaning there's no race condition:
  // no thread modifies a data that isn't its own
  // even if it is in a shared space)

  // greets[thread_number] := format("Hello from secondary thread"
  // , thread_number, " of ", thread_count)
  sprintf(shared_data->greets[private_data->thread_number]
    , "Hello from secondary thread %" PRIu64 " of %" PRIu64
    , private_data->thread_number, shared_data->thread_count);

  return NULL;
}  // end procedure
