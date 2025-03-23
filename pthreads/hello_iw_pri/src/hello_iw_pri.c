// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen <evan.chen@ucr.ac.cr> (COMMENTS FROM HELLO_W COPIED)

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// thread_private_data_t
// This is a register, or private segment of memory, for each worker thread
typedef struct private_data {
  uint64_t thread_number;  // rank
  uint64_t thread_count;  // Total number of threads
  struct private_data* next; // Pointer to next thread's private data
} private_data_t;  // By using typedef, the need for adding a struct preffix is
                   // taken care of, making it more similar to C++

/**
 * @brief Greets user in stdout. specifying which thread is used at the moment,
 *        as well as the total amount of threads in the thread team.
 * @param data: register of type private_data, which contains the thread's
 *        number and count, used inside the procedure.
 * @return NULL, no return value
 */
void* greet(void* data);

/**
 * @brief Greets user in stdout. specifying which thread is used at the moment,
 *        as well as the total number of worker threads in team for each hello
 * @param thread_count: Amount of threads to create
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(uint64_t thread_count);

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

  // Call thread creation process
  error = create_threads(thread_count);
  return error;
}  // end procedure


int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;

  // Allocate memory for threads requested,
  // along with their private data registers.
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
  
  // Use calloc() and not malloc given that this function initializes data,
  // making it useful for array allocation, though it is slower than malloc()
  private_data_t* private_data = (private_data_t*)
    calloc(thread_count, sizeof(private_data_t));

  // If both allocations were made successfully
  if (threads && private_data) {
    // for thread_number := 0 to thread_count do
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      // Store corresponding data for current thread: number and total threads
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].thread_count = thread_count;
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
    for (uint64_t thread_number = 0; thread_number < thread_count
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
      , thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  // assert(data);

  // Instead of taking the data parameter as a number, 
  // we cast it into a private_data_t to use
  private_data_t* private_data = (private_data_t*) data;

  // print "Hello from secondary thread {thread_number} of {thread_count}"
  // Here, two ways of dereferencing data from a register are used:
  // 1) Traditional approach: first get private data, then access variable
  // 2) Syntactic Sugar: Use -> arrow to get info
  // Both are valid
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , (*private_data).thread_number, private_data->thread_count);
  return NULL;
}  // end procedure
