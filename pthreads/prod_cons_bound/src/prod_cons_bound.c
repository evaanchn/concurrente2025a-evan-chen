// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// Simulates a producer and a consumer that share a bounded buffer

// @see `man feature_test_macros`
#define _DEFAULT_SOURCE  // Allows use of CLOCK_MONOTONICS

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

// Enumerate possible errors in the program
enum {
  ERR_NOMEM_SHARED = EXIT_FAILURE + 1,
  ERR_NOMEM_BUFFER,
  ERR_NO_ARGS,
  ERR_BUFFER_CAPACITY,
  ERR_ROUND_COUNT,
  ERR_MIN_PROD_DELAY,
  ERR_MAX_PROD_DELAY,
  ERR_MIN_CONS_DELAY,
  ERR_MAX_CONS_DELAY,
  ERR_CREATE_THREAD,
};

typedef struct {
  size_t thread_count;  /**< Total amount of threads. */
  size_t buffer_capacity;  /**< Size of buffer. */
  double* buffer;  /**< Actual buffer. */
  size_t rounds;  /**< Amount of rounds that buffer will be filled. */

  // Millisecond time intervals
  useconds_t producer_min_delay;  /**< Min prod time. */
  useconds_t producer_max_delay;  /**< Max prod time. */
  useconds_t consumer_min_delay;  /**< Min consume time. */
  useconds_t consumer_max_delay;  /**< Max consume time. */

  sem_t can_produce;  // Controls whether producer can produce
  sem_t can_consume;  // Determines if consumer can operate
} shared_data_t;

typedef struct  {
  size_t thread_number;  /**< Thread ID. */
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief Processes arguments specified in exec command
 * @param argc Amount of arguments passed
 * @param argv Array of arguments
 * @param shared_data Shared data struct to store
 * @return success or failure depending on args
 */
int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data);

/**
 * @brief Creates producer and consumer
 * @param shared_data Shared data for threads
 * @return Success or failure of procedure
 */
int create_threads(shared_data_t* shared_data);

void* produce(void* data);

void* consume(void* data);

useconds_t random_between(useconds_t min, useconds_t max);

int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;

  // Allocate memory for shared data
  shared_data_t* shared_data = (shared_data_t*)
    calloc(1, sizeof(shared_data_t));

  if (shared_data) {
    // First make sure arguments are valid
    error = analyze_arguments(argc, argv, shared_data);
    if (error == EXIT_SUCCESS) {
      // Initializes production semaphore as capacity of buffer
      sem_init(&shared_data->can_produce, /*pshared*/ 0,
        shared_data->buffer_capacity);
      // There's nothing to consume at the beginning
      sem_init(&shared_data->can_consume, /*pshared*/ 0, /*value*/ 0);

      // Allocate memory for buffer
      shared_data->buffer = (double*)
        calloc(shared_data->buffer_capacity, sizeof(double));
      if (shared_data->buffer) {
        // Assign seed for random number generation
        unsigned int seed = 0u;
        getrandom(&seed, sizeof(seed), GRND_NONBLOCK);
        srandom(seed);

        // Record start time
        struct timespec start_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

        error = create_threads(shared_data);

        // Record end time
        struct timespec finish_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);

        // Calculate elapsed time
        double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
          (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
        printf("execution time: %.9lfs\n", elapsed);

        // Destroy semaphores
        sem_destroy(&shared_data->can_consume);
        sem_destroy(&shared_data->can_produce);
        // Free buffer memory
        free(shared_data->buffer);
      } else {
        fprintf(stderr, "error: could not create buffer\n");
        error = ERR_NOMEM_BUFFER;
      }
    }

    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = ERR_NOMEM_SHARED;
  }

  return error;
}

int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // Must have 7 arguments
  if (argc == 7) {
    if (sscanf(argv[1], "%zu", &shared_data->buffer_capacity) != 1
      || shared_data->buffer_capacity == 0) {
        fprintf(stderr, "error: invalid buffer capacity\n");
        error = ERR_BUFFER_CAPACITY;
    } else if (sscanf(argv[2], "%zu", &shared_data->rounds) != 1
      || shared_data->rounds == 0) {
        fprintf(stderr, "error: invalid round count\n");
        error = ERR_ROUND_COUNT;
    } else if (sscanf(argv[3], "%u", &shared_data->producer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min producer delay\n");
        error = ERR_MIN_PROD_DELAY;
    } else if (sscanf(argv[4], "%u", &shared_data->producer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max producer delay\n");
        error = ERR_MAX_PROD_DELAY;
    } else if (sscanf(argv[5], "%u", &shared_data->consumer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min consumer delay\n");
        error = ERR_MIN_CONS_DELAY;
    } else if (sscanf(argv[6], "%u", &shared_data->consumer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max consumer delay\n");
        error = ERR_MAX_CONS_DELAY;
    }
  } else {
    // Inform usage to user
    fprintf(stderr, "usage: prod_cons_bound buffer_capacity rounds"
      " producer_min_delay producer_max_delay"
      " consumer_min_delay consumer_max_delay\n");
    error = ERR_NO_ARGS;
  }
  return error;
}

int create_threads(shared_data_t* shared_data) {
  assert(shared_data);
  int error = EXIT_SUCCESS;

  pthread_t producer, consumer;
  // First create producer
  error = pthread_create(&producer, /*attr*/ NULL, produce, shared_data);
  if (error == EXIT_SUCCESS) {
    // If successful, create consumer
    error = pthread_create(&consumer, /*attr*/ NULL, consume, shared_data);
    if (error != EXIT_SUCCESS) {
      fprintf(stderr, "error: could not create consumer\n");
      error = ERR_CREATE_THREAD;
    }
  } else {
    fprintf(stderr, "error: could not create producer\n");
    error = ERR_CREATE_THREAD;
  }

  // Join producer and consumer
  if (error == EXIT_SUCCESS) {
    pthread_join(producer, /*value_ptr*/ NULL);
    pthread_join(consumer, /*value_ptr*/ NULL);
  }

  return error;
}

void* produce(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;  // Get shared data
  sem_wait(&shared_data->can_produce);
  size_t count = 0;
  // For every round
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    // For every space in the buffer
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      // wait(can_produce)
      // Can only produce if there is space
      sem_wait(&shared_data->can_produce);

      // Simulate producing element
      usleep(1000 * random_between(shared_data->producer_min_delay
        , shared_data->producer_max_delay));
      // Produce element
      shared_data->buffer[index] = ++count;
      // Report produced
      printf("Produced %lg\n", shared_data->buffer[index]);

      // signal(can_consume)
      // Tells consumer there is something to consume
      sem_post(&shared_data->can_consume);
    }
  }

  return NULL;
}

void* consume(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      // wait(can_consume)
      // Waits until producer signals there is produce in buffer
      sem_wait(&shared_data->can_consume);

      double value = shared_data->buffer[index];  // Obtain produce to consume
      // Simulate consume
      usleep(1000 * random_between(shared_data->consumer_min_delay
        , shared_data->consumer_max_delay));
      printf("\tConsumed %lg\n", value);

      // signal(can_produce)
      // Tells producer a space freed up
      sem_post(&shared_data->can_produce);
    }
  }

  return NULL;
}

useconds_t random_between(useconds_t min, useconds_t max) {
  // Formula for number in a range, exclusive of upper limit
  return min + (max > min ? (random() % (max - min)) : 0);
}
