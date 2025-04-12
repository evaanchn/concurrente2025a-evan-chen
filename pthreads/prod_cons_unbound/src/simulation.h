// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef SIMULATION_H
#define SIMULATION_H

typedef struct simulation simulation_t;  // opaque record

/**
 * @brief Initializes a simulation struct, with its respective attributes.
 * @return simulation created
 */
simulation_t* simulation_create();

/**
 * @brief Initializes a simulation struct, with its respective attributes.
 * 
 * @param simulation The simulation to work with
 * @param argc Argument count
 * @param argv Arguments vector
 * @return Whether simulation was successful or not
 */
int simulation_run(simulation_t* simulation, int argc, char* argv[]);

/// @brief Deallocates memory of simulation
/// @param simulation Simulation to destroy
void simulation_destroy(simulation_t* simulation);

#endif  // SIMULATION_H
