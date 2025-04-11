// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef THREADS_H
#define THREADS_H

#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>

#include "errors.h"
#include "plate_matrix.h"

typedef struct shared_data {
  plate_matrix_t* plate_matrix;
  uint64_t thread_count;
  double mult_constant;
  double epsilon;
} shared_data_t;

typedef struct private_data {
  pthread_t thread_id;
  uint64_t starting_row;
  uint64_t ending_row;
  bool equilibrated;
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief Initializes an array of private_data_t structures.
 *
 * This function sets up the private data for each thread based on
 * the number of rows in the matrix and the desired thread count.
 * It evenly divides the work among threads and sets starting and ending
 * row indices accordingly.
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
 * @param reached_equilibrium A pointer to a flag indicating overall equilibrium.
 * @return The number of errors encountered while joining threads.
 */
int join_threads(const size_t count, private_data_t* private_data
    , bool* reached_equilibrium);

#endif  // THREADS_H
