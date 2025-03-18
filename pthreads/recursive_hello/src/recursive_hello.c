// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Edited by Evan Chen
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Recursive procedure that bids farewell and reports number if parameter received is 0, 
 * otherwise it greets and reports the number, creating another thread to call itself with 1 num less than parameter.
 * @param void* data. Void pointer that will become a size_t* to use inside procedure.
 * @return NULL
 */
void* greet(void* data);

/**
 * @brief Main procedure of program. Creates secondary thread to greet from recursively and greet itself.
 * @param void. No parameters.
 * @return EXIT_SUCCESS (0) to the O.S if nothing went wrong. EXIT_FAILURE (1) if there was an error.
 */
// procedure main:
int main(void) {
  size_t num = 2;  // Local variable initialized in 2

  // pthread creation, sending the local variable num casted to void*
  pthread_t thread;
  int error = pthread_create(&thread, /*attr*/ NULL, greet, (void*) num);

  // If thread is created successfully
  if (error == EXIT_SUCCESS) {
    // print "Hello from main thread"
    printf("Hello from main thread\n");

    // Wait for secondary thread to finish executing, if necessary
    pthread_join(thread, /*value_ptr*/ NULL);
  } else {
    fprintf(stderr, "Error: could not create secondary thread\n");
  }
  return error;
}  // end procedure

// procedure greet:
void* greet(void* data) {
  size_t number = (size_t) data;  // Casting pointer to original type.
  // Bid farewell if number is 0
  if (number == 0) {
    printf("Goodbye from secondary thread! Received number: %zu\n", number);
  } else {
    // Otherwise greet and report received number
    printf("Hello from secondary thread! Received number: %zu\n", number);

    // Create new thread to call greet again with number - 1
    pthread_t greet_thread;
    int error = pthread_create(&greet_thread, /*attr*/
      NULL, greet, (void*) number - 1);

    // If not created successfully, inform and return
    if (error != EXIT_SUCCESS) {
      fprintf(stderr, "Error: could not create secondary thread\n");
    } else {
      // Otherwise wait for new thread to finish executing
      pthread_join(greet_thread, /*value_ptr*/ NULL);
    }
  }
  return NULL;
}  // end procedure
