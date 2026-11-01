#ifndef LINALG_H
#define LINALG_H

#include <stdbool.h>

#include "error_handling.h"
#include "matrix.h"
#include "vector.h"


/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_x(matrix_t *rotmat, const double rad);

/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_y(matrix_t *rotmat, const double rad);

/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_z(matrix_t *rotmat, const double rad);

/* ------------------------------------------------------------------------- */

err_t vector_rotate(vector_t *rotated, const matrix_t rotmat, const vector_t vec);

/* ------------------------------------------------------------------------- */

err_t matrix_eigvec(const int dims, vector_t *eigenvectors, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix_eigval(const int dims, double *eigenvalues, const matrix_t mat);

/* ------------------------------------------------------------------------- */

#endif
