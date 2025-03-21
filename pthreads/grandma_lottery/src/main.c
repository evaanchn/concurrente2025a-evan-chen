// Copyright <2025> <Evan Chen> CC-BY-4

#define _POSIX_C_SOURCE 199506L 

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void* buy_lottery(void* data);

// procedure main:
int main(void) {
  // create_thread(buy_lottery)
  pthread_t thread_grand_child1, thread_grand_child2;
  const int error = pthread_create(&thread_grand_child1,
      /*attr*/ NULL, buy_lottery , /*arg*/ NULL);

  const int error2 = pthread_create(&thread_grand_child2,
      /*attr*/ NULL, buy_lottery , /*arg*/ NULL);

  if (error == EXIT_SUCCESS && error2 == EXIT_SUCCESS) {
    void* number1 = NULL;
    void* number2 = NULL;
    pthread_join(thread_grand_child1, &number1);
    pthread_join(thread_grand_child2, &number2);

    printf("My numbers are %zu and %zu \n", (size_t) number1, (size_t) number2);
  } else {
    fprintf(stderr, "Error: could not create secondary thread\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}  // end procedure

// procedure buy_lottrey:
void* buy_lottery(void* data) {
  (void)data;
  sleep(1);
  // int number = rand() % 100;
  // return &number;

  // By using current time and thread ID, we make sure that numbers are random
  // at each execution
  unsigned seed = time(NULL) + clock() + pthread_self();

  // rand_r() takes the seed to generate a psuedo random number
  size_t number = rand_r(&seed) %100;
  return (void*) number;
}  // end procedure
