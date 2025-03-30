// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef PLATE_H
#define PLATE_H

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "plate_matrix.h"

/**
 * @struct plate_t
 * @brief Structure to store plate properties and state.
 */
typedef struct {
  char* file_name;               ///< Name of the plate file
  plate_matrix_t* plate_matrix;  ///< Pointer to plate matrix structure
  double thermal_diffusivity;    ///< Thermal diffusivity coefficient
  uint64_t interval_duration;    ///< Time step interval
  uint64_t cells_dimension;      ///< Cell size dimension
  double epsilon;                ///< Threshold for equilibrium check
  uint64_t k_states;             ///< Current simulation state
} plate_t;

/**
 * @brief Loads the plate matrix from a binary file.
 * 
 * Reads the matrix dimensions and data from the file into a plate structure.
 * 
 * @param plate Pointer to the plate structure.
 * @param source_directory Directory containing the plate file.
 * @return EXIT_SUCCESS on success, error code otherwise.
 */
int set_plate_matrix(plate_t* plate, char* source_directory);

/**
 * @brief Updates the plate's temperature matrix using diffusion calculations.
 * 
 * Computes new temperatures for each cell and checks for equilibrium.
 * 
 * @param plate Pointer to the plate structure.
 * @return True if equilibrium is reached, false otherwise.
 */
bool update_plate(plate_t* plate);

/**
 * @brief Writes the updated plate matrix to a binary file.
 * 
 * Saves the new state of the plate with a filename reflecting the state count.
 * 
 * @param plate Pointer to the plate structure.
 * @param source_directory Directory where the file should be saved.
 * @return EXIT_SUCCESS on success, error code otherwise.
 */
int update_plate_file(plate_t* plate, char* source_directory);

/**
 * @brief Generates a filename for the updated plate state.
 * 
 * Appends the state count as a suffix to the original filename.
 * 
 * @param plate Pointer to the plate structure.
 * @return Newly allocated string containing the updated filename.
 */
char* set_plate_file_name(plate_t* plate);

#endif  // PLATE_H
