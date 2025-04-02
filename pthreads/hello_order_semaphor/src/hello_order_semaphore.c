// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen

#define _POSIX_C_SOURCE 199506L

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// thread_shared_data_t
typedef struct shared_data {
  // Array of semaphores, where only one is on at a time,
  // assuring orderly printing and indeterminism at once
  sem_t* can_greet;
  uint64_t thread_count;  // Total amount of threads in team
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // rank
  shared_data_t* shared_data;
} private_data_t;


/**
 * @brief Greets user in stdout. specifying which thread is used at the moment,
 *        as well as the total amount of threads in the thread team.
 *        The greetings will appear in order, from lowest to greatest number.
 * @param data: register of type private_data, which contains the thread's
 *        number, and a shared data register with a reference to the sem array
 *        and thread count
 * @return NULL, no return value
 */
void* greet(void* data);

/**
 * @brief Creates and later joins threads used to call greet. Also in charge
 *        of setting up array of semaphores and its destruction
 * @param shared_data: Amount of threads to create, but now in a register that
 *        all threads can access, as well as a reference to the semaphore array
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(shared_data_t* shared_data);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
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
    // Allocate semaphore array memory
    shared_data->can_greet = (sem_t*) calloc(thread_count, sizeof(sem_t));
    shared_data->thread_count = thread_count;

    // Initialize first semaphore in 1, the rest in 0. This sets up the cyclical
    // activations of each semaphore.
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // can_greet[thread_number] := create_semaphore(not thread_number)
      error = sem_init(&shared_data->can_greet[thread_number], /*pshared*/ 0
        , /*value*/ !thread_number);
    }

    // If memory for can_greet was initialized successfully
    if (shared_data->can_greet) {
      // Record start time
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      // Call create threads to take care of thread creation and joining
      error = create_threads(shared_data);

      // Record end time
      clock_gettime(CLOCK_MONOTONIC, &finish_time);
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      // Report duration of execution
      printf("Execution time: %.9lfs\n", elapsed_time);

      // Free memory for semaphore array
      free(shared_data->can_greet);
    } else {
      fprintf(stderr, "Error: could not allocate semaphores\n");
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
  pthread_t* threads = (pthread_t*)
    malloc(shared_data->thread_count * sizeof(pthread_t));
  // Allocate memory for private data
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));

  // If allocation was successful for both
  if (threads && private_data) {
    // Send out threads
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      if (error == EXIT_SUCCESS) {
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

    // Join threads and destroy all semaphores
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
      sem_destroy(&shared_data->can_greet[thread_number]);
    }

    // Free allocated memory
    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 23;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  assert(data);
  // Obtain data
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  // Wait until it is my turn
  printf("  %" PRIu64 "/%" PRIu64 ": wait for semaphore\n"
    , private_data->thread_number, shared_data->thread_count);

  // wait(can_greet[thread_number]) Decrements current semaphore from 1 to 0
  // Each thread waits for their corresponding semaphore to be habilitated
  int error = sem_wait(&shared_data->can_greet[private_data->thread_number]);
  if (error) {
    fprintf(stderr, "error: could not wait for semaphore\n");
  }

  // print "Hello from secondary thread {thread_number} of {thread_count}"
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count);

  // Allow subsequent thread to do the task
  // signal(can_greet[(thread_number + 1) mod thread_count])
  // This ensures last thread will not accidentally access out of bounds memory,
  // Instead it will reset first semaphore to 1
  const uint64_t next_thread = (private_data->thread_number + 1)
    % shared_data->thread_count;

  // Increment next thread's semaphore, allowing it to print
  error = sem_post(&shared_data->can_greet[next_thread]);
  if (error) {
    fprintf(stderr, "error: could not increment semaphore\n");
  }

  return NULL;
}  // end procedure
