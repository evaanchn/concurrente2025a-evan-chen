// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

typedef struct queue_node {
  size_t data;
  struct queue_node* next;
} queue_node_t;

typedef struct {
  pthread_mutex_t can_access_queue;
  queue_node_t* head;
  queue_node_t* tail;
} queue_t;

/**
 * @brief Initializes queue and queue access mutex
 * @param queue Queue to work with
 * @remarks This subroutine is NOT thread-safe
 * @return Success or failure of procedure
 */
int queue_init(queue_t* queue);

/// @brief Destroys queue and related dynamic memory
/// @see queue_init
int queue_destroy(queue_t* queue);


/**
 * @brief Checks whether the queue is empty.
 *
 * @param queue Pointer to the queue to check.
 * @return true if the queue is empty, false otherwise.
 */
bool queue_is_empty(queue_t* queue);

/**
 * @brief Adds a new element to the end of the queue.
 *
 * Allocates a new node and appends it to the queue in a thread-safe manner.
 *
 * @param queue Pointer to the queue.
 * @param data The data to enqueue.
 * @return EXIT_SUCCESS on success, or EXIT_FAILURE if memory allocation fails.
 */
int queue_enqueue(queue_t* queue, const size_t data);

/// @brief Removes the front element from the queue and retrieves its data
/// @see queue_enqueue
int queue_dequeue(queue_t* queue, size_t* data);

/**
 * @brief Clears all elements from the queue.
 *
 * Frees all nodes and resets the queue to an empty state.
 * The function is thread-safe.
 *
 * @param queue Pointer to the queue.
 */
void queue_clear(queue_t* queue);


#endif  // QUEUE_H