// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef PLATE_MATRIX_H
#define PLATE_MATRIX_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

/** 
 * @struct plate_matrix_t
 * @brief Represents a heat diffusion plate matrix.
 */
typedef struct {
    uint64_t rows;          /**< Number of rows in the matrix */
    uint64_t cols;          /**< Number of columns in the matrix */
    double* matrix;        /**< Pointer to the primary matrix */
    double* auxiliary_matrix; /**< Pointer to the auxiliary matrix */
} plate_matrix_t;

/**
 * @brief Initializes a plate matrix with specified dimensions.
 * 
 * Allocates memory for both primary and auxiliary matrices.
 * 
 * @param rows Number of rows.
 * @param cols Number of columns.
 * @return Pointer to allocated plate_matrix_t or NULL on failure.
 */
plate_matrix_t* init_plate_matrix(uint64_t rows, uint64_t cols);

/**
 * @brief Initializes the auxiliary matrix by copying boundary values.
 * 
 * Copies borders ensuring the boundary conditions remain constant.
 * 
 * @param plate_matrix Pointer to the plate matrix.
 */
void init_auxiliary(plate_matrix_t* plate_matrix);

/**
 * @brief Swaps the primary and auxiliary matrices. Used to update the
 * simulation step by step.
 * @param plate_matrix Pointer to the plate matrix.
 */
void set_auxiliary(plate_matrix_t* plate_matrix);

/**
 * @brief Updates a single cell based on its neighboring temperatures.
 * @param plate_matrix Pointer to the plate matrix.
 * @param row Row index.
 * @param col Column index.
 * @param mult_constant Multiplication constant for heat diffusion.
 */
void update_cell(plate_matrix_t* plate_matrix, uint64_t row,
    uint64_t col, double mult_constant);

/**
 * @brief Frees memory allocated for both matrices in plate_matrix 
 * and then plate_matrix
 * @param plate_matrix Pointer to the plate matrix.
 */
void destroy_plate_matrix(plate_matrix_t* plate_matrix);


#endif  // PLATE_MATRIX_H
