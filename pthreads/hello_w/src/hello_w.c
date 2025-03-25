// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen <evan@ucr.ac.cr>
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Greets user in stdout. specifying which thread is used at the moment
 * @param data: the thread rank (uint64_t), casted to void* for thread to use
 * @return NULL, no return value
 */
void* greet(void* data);

/**
 * @brief Creates and later joins threads used to call greet, sending necessary
 *        arguments to each thread.
 * @param thread_count: Amount of threads to create
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(uint64_t thread_count);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
// The #if #endif combo controls whether the compiler will receive the code
// by telling preprocessor not to include it with the 0 condition
#if 0
  // A for loop that prints the arguments
  // passed via invocation command in terminal
  for (int index = 0; index < argc; ++index) {
    printf("argv[%d] = '%s'\n", index, argv[index]);
  }
  return 0;
#endif

  int error = EXIT_SUCCESS;

  // Start by assuming that thread_count will be the amount of available cores
  // Extracted with the sysconf() function,
  // which takes the identifier for available cores as argument
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);

  // If user specifes an amount upon program execution
  if (argc == 2) {
    // substitute thread_count with said number, using sscanf to read
    // from argument vectors second element
    // (first argument will be the execution command). SCNu64 is used
    // to format the number read.
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      // Indicate error if an input error occurred
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  // Call thread creation process
  error = create_threads(thread_count);
  return error;
}  // end procedure


int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;  // Error control, assume success

  // Allocate memory for a pthread array in heap with malloc():
  // amount of threads requested * 8 bytes (size of a pthread).
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));

  // If memory was allocated successfully
  if (threads) {
    // for thread_number := 0 to thread_count do
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      // "Hire" worker to go greet user, passing the current thread_number,
      // casted as void*. create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ (void*) thread_number);

      // If current thread was created unsuccessfully
      if (error == EXIT_SUCCESS) {
      } else {
        // Print error and break from threads creation
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    // For every thread requested, join after completing job
    // This avoids the serialization of a concurrent code
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      // Will wait for secondary thread if it has not finished greeting
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    free(threads);
  } else {
    // Notify inability to allocate thread_count amount of threads
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  // To allow release versions of code (for clients), we comment assert,
  // given that this is a programmer error
  // assert(data);

  // Casts parameter data from (void*) to the actual number of the thread, rank
  // The type is uint64_t, or an unsigned int of 64 bits.
  // This is to specify the size of the data at compiling time.
  const uint64_t rank = (uint64_t) data;
  // print "Hello from secondary thread $(rank)"
  // The print format specifier for uint64_t is PRIu64
  printf("Hello from secondary thread %" PRIu64 "\n", rank);
  return NULL;
}  // end procedure

