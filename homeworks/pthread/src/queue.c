// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// TAKEN FROM PROD_CONS_UNBOUND

#include <assert.h>
#include <stdlib.h>

#include "queue.h"

/// @brief Unsafe version of removing first element (no mutex)
/// @param queue Queue to work with
void queue_remove_first_unsafe(queue_t* queue);

/// @brief Checks if queue is empty with no mutex involvement
/// @param queue Queue to check
/// @return Empty (true) or not (false)
bool queue_is_empty_unsafe(queue_t* queue);

int queue_init(queue_t* queue) {
  assert(queue);
  // Initialize the mutex to protect access to the queue
  int error = pthread_mutex_init(&queue->can_access_queue, NULL);
  // Set both head and tail to NULL indicating an empty queue
  queue->head = NULL;
  queue->tail = NULL;
  return error;
}

int queue_destroy(queue_t* queue) {
  // Clear any remaining nodes before destroying the mutex
  queue_clear(queue);
  return pthread_mutex_destroy(&queue->can_access_queue);
}

bool queue_is_empty(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  // Queue is empty if head is NULL
  bool result = queue->head == NULL;
  pthread_mutex_unlock(&queue->can_access_queue);
  return result;
}

bool queue_is_empty_unsafe(queue_t* queue) {
  assert(queue);
  // Same as queue_is_empty but does not acquire a lock
  // Should only be used when mutex is already held
  return queue->head == NULL;
}

int queue_enqueue(queue_t* queue, const uint64_t data) {
  assert(queue);
  int error = EXIT_SUCCESS;

  // Allocate a new node to store the data
  queue_node_t* new_node = (queue_node_t*) calloc(1, sizeof(queue_node_t));
  if (new_node) {
    new_node->data = data;

    pthread_mutex_lock(&queue->can_access_queue);
    if (queue->tail) {
      // Add new node at the end and update tail
      queue->tail = queue->tail->next = new_node;
    } else {
      // Queue was empty; head and tail both point to the new node
      queue->head = queue->tail = new_node;
    }
    pthread_mutex_unlock(&queue->can_access_queue);
  } else {
    // Allocation failed
    error = EXIT_FAILURE;
  }

  return error;
}

int queue_dequeue(queue_t* queue, uint64_t* data) {
  assert(queue);
  int error = 0;

  pthread_mutex_lock(&queue->can_access_queue);
  if (!queue_is_empty_unsafe(queue)) {
    // If caller wants the data, copy it
    if (data) {
      *data = queue->head->data;
    }
    // Remove the first node from the queue
    queue_remove_first_unsafe(queue);
  } else {
    // Queue is empty, can't dequeue
    error = EXIT_FAILURE;
  }
  pthread_mutex_unlock(&queue->can_access_queue);

  return error;
}

void queue_remove_first_unsafe(queue_t* queue) {
  assert(queue);
  assert(!queue_is_empty_unsafe(queue));

  // Remove and free the node at the front of the queue
  queue_node_t* node = queue->head;
  queue->head = queue->head->next;
  free(node);

  // If head becomes NULL, reset tail too
  if (queue->head == NULL) {
    queue->tail = NULL;
  }
}

void queue_clear(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  // Repeatedly remove the first node until the queue is empty
  while (!queue_is_empty_unsafe(queue)) {
    queue_remove_first_unsafe(queue);
  }
  pthread_mutex_unlock(&queue->can_access_queue);
}
