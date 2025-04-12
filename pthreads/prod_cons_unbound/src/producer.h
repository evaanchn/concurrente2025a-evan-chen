// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef PRODUCER_H
#define PRODUCER_H

/// @brief While products are not completed, it produces data to put in queue
/// @param data The simulation struct with the data
void* produce(void* data);

#endif  // PRODUCER_H
