// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef THREADS_H
#define THREADS_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>

#include "errors.h"
#include "plate.h"
#include "plate_matrix.h"

typedef struct shared_data {
  plate_matrix_t* plate_matrix; /**< Plate matrix being equilibrated */
  uint64_t thread_count;        /**< Total amount of threads */
  double mult_constant;         /**< Constant in new temp formula */
  double epsilon;               /**< Epsilon associated to the plate */
  bool equilibrated_plate;      /**< True if plate was equilibrated */
  pthread_mutex_t can_access_equilibrated; /**< Controls access to flag */
  pthread_barrier_t can_continue1;  /**< First barrier */
  pthread_barrier_t can_continue2;  /**< Second barrier */
  uint64_t k_states;              /**< The amount of states iterated */
} shared_data_t;

typedef struct private_data {
  pthread_t thread_id;         /**< POSIX thread ID. */
  uint64_t starting_row;       /**< Index of the first row assigned to thread */
  uint64_t finish_row;         /**< Index of the last row assigned to thread */
  bool equilibrated;           /**< Indicates if section reached equilibrium */
  shared_data_t* shared_data;  /**< Pointer to the shared data structure. */
} private_data_t;

/**
 * @brief Initializes a shared data struct
 *
 * This procedure sets up a shared data struct by initializing values
 * and concurrency control tools.
 *
 * @param shared_data The shared data struct to initialize
 * @param plate Plate to equilibrate, with important information for the struct
 * @param thread_count Amount of threads requested from args.
 * @return Success or failure of the intialization.
 */
int init_shared_data(shared_data_t* shared_data, plate_t* plate
    , uint64_t thread_count);

/**
 * @brief Initializes an array of private_data_t structures.
 *
 * This function sets up the private data for each thread based on
 * the number of rows in the matrix and the desired thread count.
 * It evenly divides the work among threads and sets starting and ending
 * row indices accordingly.
 *
 * @param data  A pointer to the shared_data_t structure.
 * @return A pointer to an array of initialized private_data_t structures,
 *         or NULL on failure.
 */
private_data_t* init_private_data(void* data);

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

#endif  // THREADS_H
