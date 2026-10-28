#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "error_handling.h"


typedef struct {
    double *values;
    size_t rows;
    size_t cols;
} matrix_t;


/* ------------------------------------------------------------------------- */

err_t matrix_init(matrix_t *mat, const int rows, const int cols);

/* ------------------------------------------------------------------------- */

err_t matrix_init_identity(matrix_t *mat, const int dims);

/* ------------------------------------------------------------------------- */

err_t matrix_copy(matrix_t *dst, const matrix_t src);

/* ------------------------------------------------------------------------- */

void matrix_free(matrix_t *mat);

/* ------------------------------------------------------------------------- */

bool matrix_is_allocated(const matrix_t *mat);

/* ------------------------------------------------------------------------- */

bool matrix_is_square(const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix_print(char *msg, const int msg_len, const matrix_t mat);

/* ------------------------------------------------------------------------- */

double matrix_get(const matrix_t mat, const int row, const int col);

/* ------------------------------------------------------------------------- */

err_t matrix_set(matrix_t *mat, const int row, const int col, const double value);

/* ------------------------------------------------------------------------- */

err_t matrix_fill(matrix_t *mat, const double fill_with);

/* ------------------------------------------------------------------------- */

err_t matrix_scale(matrix_t *mat, const double scale);

/* ------------------------------------------------------------------------- */

err_t matrix_add(matrix_t *sum, const matrix_t mat1, const matrix_t mat2);

/* ------------------------------------------------------------------------- */

err_t matris_sub(matrix_t *mat1_minus_mat2, const matrix_t mat1, const matrix_t mat2);

/* ------------------------------------------------------------------------- */

err_t matrix_transpose(matrix_t *mat_T, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix2_mul(matrix_t *axb, const matrix_t a, const matrix_t b);

/* ------------------------------------------------------------------------- */

err_t matrix3_mul(matrix_t *axb, const matrix_t a, const matrix_t b);

/* ------------------------------------------------------------------------- */

err_t matrix3_mulmul(matrix_t *axbxc, const matrix_t a, const matrix_t b, const matrix_t c);

/* ------------------------------------------------------------------------- */

err_t matrix2_det(double *det, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix3_det(double *det, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix2_inv(matrix_t *inverse, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix3_inv(matrix_t *inverse, const matrix_t mat);

/* ------------------------------------------------------------------------- */

bool matrix_equal(const matrix_t mat1, const matrix_t mat2, const double tolerance);

/* ------------------------------------------------------------------------- */

#endif
