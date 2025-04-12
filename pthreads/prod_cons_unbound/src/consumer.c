// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "consumer.h"

void* consume(void* data) {
  simulation_t* simulation = (simulation_t*)data;

  // Same as production, use while true approach
  while (true) {
    // lock(can_access_consumed_count)
    pthread_mutex_lock(&simulation->can_access_consumed_count);
    // If there are still products unclaimed, claim
    if (simulation->consumed_count < simulation->unit_count) {
      // Reserve the next product to me
      ++simulation->consumed_count;
    } else {
      // unlock(can_access_consumed_count)
      pthread_mutex_unlock(&simulation->can_access_consumed_count);
      // break while
      break;
    }
    // unlock(can_access_consumed_count)
    pthread_mutex_unlock(&simulation->can_access_consumed_count);

    // wait(can_consume)
    // This ensures that thread will only consume until there is product
    sem_wait(&simulation->can_consume);

    size_t value = 0;
    // Obtain value to consume
    queue_dequeue(&simulation->queue, &value);
    printf("\tConsuming %zu\n", value);
    // Simulate consumption
    usleep(1000 * random_between(simulation->consumer_min_delay
      , simulation->consumer_max_delay));
  }

  return NULL;
}
