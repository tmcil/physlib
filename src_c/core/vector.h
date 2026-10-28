
#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "error_handling.h"


typedef struct {
    double *values;
    size_t dims;
} vector_t;


/* ------------------------------------------------------------------------- */

err_t vector_init(vector_t *vec, const int dims);

/* ------------------------------------------------------------------------- */

err_t vector_copy(vector_t *dst, const vector_t src);

/* ------------------------------------------------------------------------- */

void vector_free(vector_t *vec);

/* ------------------------------------------------------------------------- */

bool vector_is_allocated(const vector_t *vec);

/* ------------------------------------------------------------------------- */

err_t vector_print(const vector_t vec);

/* ------------------------------------------------------------------------- */

double vector_get(const vector_t vec, const int index);

/* ------------------------------------------------------------------------- */

err_t vector_set(vector_t *vec, const int index, const double value);

/* ------------------------------------------------------------------------- */

err_t vector_fill(vector_t *vec, const double fill_with);

/* ------------------------------------------------------------------------- */

err_t vector_scale(vector_t *vec, const double scale);

/* ------------------------------------------------------------------------- */

err_t vector_add(vector_t *sum, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

err_t vector_sub(vector_t *vec1_minus_vec2, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

err_t vector_norm(double *norm, const vector_t vec);

/* ------------------------------------------------------------------------- */

err_t vector_unit(vector_t *unit, const vector_t vec);

/* ------------------------------------------------------------------------- */

err_t vector_dot(double *dot_product, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

err_t vector3_cross(vector_t *cross, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

bool vector_equal(const vector_t vec1, const vector_t vec2, const double tolerance);

/* ------------------------------------------------------------------------- */

#endif
