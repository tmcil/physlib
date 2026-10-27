#ifndef LINALG_H
#define LINALG_H

#include "matrix.h"
#include "vector.h"


/* ------------------------------------------------------------------------- */

int vector_rotate(vector_t *rotated, const matrix_t rotmat, const vector_t vec);

/* ------------------------------------------------------------------------- */

int matrix_eigvec(const int dims, vector_t *eigenvectors, const matrix_t mat);

/* ------------------------------------------------------------------------- */

int matrix_eigval(const int dims, double *eigenvalues, const matrix_t mat);

/* ------------------------------------------------------------------------- */

#endif
