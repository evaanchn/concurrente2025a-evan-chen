// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef THREADS_H
#define THREADS_H

#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "errors.h"
#include "plate.h"
#include "queue.h"

typedef struct shared_data {
  plate_matrix_t* plate_matrix; /**< Plate matrix being equilibrated */
  uint64_t thread_count;        /**< Total amount of threads */
  double mult_constant;         /**< Constant in new temp formula */
  double epsilon;               /**< Epsilon associated to the plate */
  queue_t rows_queue;          /**< Thread safe queue with indicator  */
  sem_t can_get_working_row;    /**< Permits thread to dequeue */
  uint64_t stop_condition;      /**< To terminate thread once cocnsumed */
  bool equilibrated_plate;      /**< Indicates if plate has been equilibrated */
  pthread_mutex_t can_access_equilibrated; /*< Mutex for equilibrated plate */
  sem_t state_done;             /**< Permits main thread to continue balance */
} shared_data_t;

typedef struct private_data {
  pthread_t thread_id;         /**< POSIX thread ID. */
  shared_data_t* shared_data;  /**< Pointer to the shared data structure. */
} private_data_t;

/// @brief Intializes a shared data struct with its attributes
/// @param shared_data The struct to initialize
/// @param thread_count The amount of threads to set
/// @return EXIT_SUCCESS if succeeded, else, error code
int init_shared_data(shared_data_t* shared_data, uint64_t thread_count);

/**
 * @brief Initializes an array of private_data_t structures.
 *
 * This function initializes and sets up the private data for each thread
 * by adding the shared data pointer to the array
 *
 * @param count The number of threads requested.
 * @param data  A pointer to the shared_data_t structure.
 * @return A pointer to an array of initialized private_data_t structures,
 *         or NULL on failure.
 */
private_data_t* init_private_data(const size_t count, void* data);

/**
 * @brief Creates and starts threads using the specified routine.
 *
 * Iterates through each thread and assigns it the routine and data
 * to execute. On failure, it joins already created threads and returns
 * an error code.
 *
 * @param routine The function each thread should execute.
 * @param data    A pointer to the private_data_t array.
 * @return EXIT_SUCCESS on success, ERR_CREATE_THREAD on failure.
 */
int create_threads(void* (*routine)(void*), void* data);

/**
 * @brief Joins a given number of threads and updates equilibrium status.
 *
 * Waits for all threads to complete. Checks if all threads reached
 * equilibrium and updates the status accordingly.
 *
 * @param count               The number of threads to join.
 * @param private_data        A pointer to the array of private_data_t.
 * @return The number of errors encountered while joining threads.
 */
int join_threads(const size_t count, private_data_t* private_data);

/// @brief Destroys a shared data struct with its attributes
/// @param shared_data The struct to initialize
void destroy_shared_data(shared_data_t* shared_data);

#endif  // THREADS_H
