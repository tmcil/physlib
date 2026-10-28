#ifndef LINALG_H
#define LINALG_H

#include "error_handling.h"
#include "matrix.h"
#include "vector.h"

/* ------------------------------------------------------------------------- */

err_t vector_rotate(vector_t *rotated, const matrix_t rotmat, const vector_t vec);

/* ------------------------------------------------------------------------- */

err_t matrix_eigvec(const int dims, vector_t *eigenvectors, const matrix_t mat);

/* ------------------------------------------------------------------------- */

err_t matrix_eigval(const int dims, double *eigenvalues, const matrix_t mat);

/* ------------------------------------------------------------------------- */

#endif
