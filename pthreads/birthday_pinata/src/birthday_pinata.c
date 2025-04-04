// Copyright 2025 Evan Chen <evan.chen@ucr.ac.cr> CC-BY 4.0
// CODE BASE FROM position_race

#define _XOPEN_SOURCE 500  // For usleep

#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// thread_shared_data_t
typedef struct shared_data {
  // How many hits the pinata can take
  uint64_t hit_resistance;

  // Initialized at pinata's hit resistance, will be reduced until gets to 0
  int64_t pinata;

  // Mutual exclusion to control who gets to hit the pinata
  pthread_mutex_t can_hit_pinata;

  // Total amount of threads invited
  uint64_t thread_count;
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // thread identifier
  uint64_t hits;  // Record of hits
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief While pinata has not burst, hits pinata once every turn
 * @param data, private_data of the thread
 */
void* hit_pinata(void* data);

/**
 * @brief Creates and later joins threads that call hit_pinata
 * @param shared_data: Amount of threads to create, pinata, hit_resistance
 *        and the mutex for hitting the pinata
 * @return error: Control variable that indicates success or failure of process
 */
int create_threads(shared_data_t* shared_data);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;

  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);  // Default
  uint64_t pinata_resistance = 10;  // Default pinata resistance
  if (argc == 3) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }

    if (sscanf(argv[2], "%" SCNu64, &pinata_resistance) == 1) {
    } else {
      fprintf(stderr, "Error: invalid pinata resistance\n");
      return 12;
    }
  }

  // Initialize shared_data
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) {
    // INITIALIZATION OF MUTEX: pass address of mutex in shared_data
    error = pthread_mutex_init(&shared_data->can_hit_pinata, /*attr*/NULL);

    if (error == EXIT_SUCCESS) {
      shared_data->thread_count = thread_count;
      shared_data->hit_resistance = pinata_resistance;
      shared_data->pinata = pinata_resistance;
      error = create_threads(shared_data);

      // Free mutex and shared_data
      pthread_mutex_destroy(&shared_data->can_hit_pinata);
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
      private_data[thread_number].hits = 0;
      private_data[thread_number].shared_data = shared_data;
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, hit_pinata
        , /*arg*/ &private_data[thread_number]);
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // Loop to join the threads
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    // Free memory for private data array and threads array
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
void* hit_pinata(void* data) {
  assert(data);

  // Obtain private and shared data
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  bool broke_pinata = false;
  while (1) {
    /// lock(can_hit_pinata)
    pthread_mutex_lock(&shared_data->can_hit_pinata);
    // Prevent threads from hitting a broken pinata
    if (!shared_data->pinata) {
      pthread_mutex_unlock(&shared_data->can_hit_pinata);
      break;
    }

    --shared_data->pinata;  // Hit pinata once
    ++private_data->hits;   // Increment hits recorded

    // if the pinata is not 0, broke_pinata will continue being false
    // else, it will mean the current thread broke the pinata
    broke_pinata = shared_data->pinata ? false : true;

    // unlock(can_hit_pinata)
    pthread_mutex_unlock(&shared_data->can_hit_pinata);

    if (broke_pinata) break;  // If pinata broke, no need to hit again

    usleep(1);  // Sleeps to prevent thread from returning too soon
  }

  // Report results accordingly
  if (broke_pinata) {
    printf("Thread %" PRIu64 "/%" PRIu64 ": %" PRIu64
      "hits, I broke the pinata\n", private_data->thread_number,
      shared_data->hit_resistance, private_data->hits);
  } else {
    printf("Thread %" PRIu64 "/%" PRIu64 ": %" PRIu64 "hits\n",
      private_data->thread_number, shared_data->hit_resistance,
      private_data->hits);
  }

  return NULL;
}  // end procedure
