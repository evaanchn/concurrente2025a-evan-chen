// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen <evan.chen@ucr.ac.cr>
// hello_iw_shr.c comments used

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
  uint64_t next_thread; // Next thread allowed to greet is added
  uint64_t thread_count;
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
 *        number and count, used inside the procedure. Count and next allowed 
 *        thread are stored in a shared data register.
 * @return NULL, no return value
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
      return 11;  // Error code that indicates first error of first procedure 
                  // Return codes can be obtained in command line with 'echo $?'
    }
  }

  // Allocate memory for a shared data register
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));

  // If successfully created
  if (shared_data) {
    // NEW: next thread allowed to greet is initialized as 0
    shared_data->next_thread = 0;

    // Store thread count in shared_data for all threads to access
    shared_data->thread_count = thread_count;

    // Timespec stores seconds an nanoseconds. We need two of these to record
    // start and end time of the create_threads() procedure
    struct timespec start_time, finish_time;

    // CLOCK_MONOTONIC is an unsettable clock that represents monotonic clock,
    // ticking stably since an unspecified time. First store start time.
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    error = create_threads(shared_data);

    // Upong completion, record end time of process
    clock_gettime(CLOCK_MONOTONIC, &finish_time);

    // Elapsed time is obtained by subtracting start time from end time
    double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;  
      // Nanosecond convert. 1e-9 is used to avoid numbers being taken as whole

    // Print execution time
    printf("Execution time: %.9lfs\n", elapsed_time);

    // Deallocate memory for shared data
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    return 12;
  }
  return error;
}  // end procedure

int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;

  // Allocate memory for threads requested,
  // along with their private data registers.
  pthread_t* threads = (pthread_t*) malloc(shared_data->thread_count * sizeof(pthread_t));
  
  // Use calloc() and not malloc given that this function initializes data,
  // making it useful for array allocation, though it is slower than malloc()
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));

  // If both allocations were made successfully
  if (threads && private_data) {
    // for thread_number := 0 to thread_count do
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // Store corresponding data for current thread: number & shared data
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].shared_data = shared_data;
      // create_thread(greet, private_data[thread_number])
      // Notice that we send the register's ADDRESS and not the register
      // otherwise more bytes would be sent than allowed
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ &private_data[thread_number]);
      
        // If one of the threads could not be created
      if (error == EXIT_SUCCESS) {
      } else {
        // Report error and return unique error code 21
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

     // For every thread requested, join after completing job
    // This avoids the serialization of a concurrent code
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // Will wait for secondary thread if it has not finished greeting
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    // Free memory
    free(private_data);
    free(threads);
  } else {
    // Print error if could not allocate n amount of threads
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  assert(data);  // The data passed as parameter can no longer be NULL

  // Cast private data and obtain shared data from private data
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  // Wait until it is my turn with an active while (DANGEROUS)
  // Takes up all of CPU's resources, overworks scheduler when
  // there are many threads actively waiting with this while
  while (shared_data->next_thread < private_data->thread_number) {
    // busy-waiting
  }  // end while

  // print "Hello from secondary thread {thread_number} of {thread_count}"
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count);

  // Allow subsequent thread to do the task
  ++shared_data->next_thread;

  return NULL;
}  // end procedure
