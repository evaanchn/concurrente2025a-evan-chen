// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0
// Commented by Evan Chen 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>

/**
 * @brief Greets user in standard output. Called upon with secondary thread.
 * @param void* data. Generic type data that is unused inside the procedure 
 * and allows thread usage.
 * @return NULL, given it's a procedure.
 */
void* greet(void* data);

/**
 * @brief Main procedure of program. Creates secondary thread to greet from 
 * and greets in standard output itself.
 * @param void. No parameters.
 * @return EXIT_SUCCESS (0) to the O.S if nothing went wrong. 
 * EXIT_FAILURE (1) if there was an error.
 */
// procedure main:
int main(void) {
  // create_thread(greet)
  pthread_t thread;
  // pthread_create initializes the thread, giving no particular priority
  // upon passing the greet subroutine's address (without calling it)
  // Given that the greet procedure does not have parameters,
  // the default argument is NULL.
  int error = pthread_create(&thread, /*attr*/ NULL, greet, /*arg*/ NULL);

  // To ensure that the secondary thread was created successfully, 
  // compare the code returned with EXIT_SUCCESS
  if (error == EXIT_SUCCESS) { 
    // usleep(1);  
    // indeterminism: We cannot predict how the threads are going to behave
    // or how they will be organized inside the machine
    // Making the main thread sleep for a period of time could force 
    // the secondary thread to greet first

    // print "Hello from main thread"
    printf("Hello from main thread\n"); 

    // Free secondary thread if it has finished executing. 
    // If not, it waits for the thread to terminate first.
    pthread_join(thread, /*value_ptr*/ NULL); 
  } else { 
    // If thread creation was not successful, print error message in stderr.
    fprintf(stderr, "Error: could not create secondary thread\n");
  }
  return error;
}  // end procedure

// procedure greet:
void* greet(void* data) {
  // This line indicates that the data parameter is purposefully unused: 
  // it silences a potential "variable not used" warning.
  (void)data;
  // print "Hello from secondary thread"
  printf("Hello from secondary thread\n");
  return NULL;
}  // end procedure

