
#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


typedef struct {
    double *values;
    int dims;
} vector_t;


/* ------------------------------------------------------------------------- */

int vector_init(vector_t *vec, const int dims);

/* ------------------------------------------------------------------------- */

int vector_copy(vector_t *dst, const vector_t src);

/* ------------------------------------------------------------------------- */

void vector_free(vector_t *vec);

/* ------------------------------------------------------------------------- */

bool vector_is_allocated(const vector_t *vec);

/* ------------------------------------------------------------------------- */

int vector_print(char *msg, const int msg_len, const vector_t vec);

/* ------------------------------------------------------------------------- */

double vector_get(const vector_t vec, const int index);

/* ------------------------------------------------------------------------- */

int vector_set(vector_t *vec, const int index, const double value);

/* ------------------------------------------------------------------------- */

int vector_fill(vector_t *vec, const double fill_with);

/* ------------------------------------------------------------------------- */

int vector_scale(vector_t *vec, const double scale);

/* ------------------------------------------------------------------------- */

int vector_add(vector_t *sum, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

int vector_sub(vector_t *vec1_minus_vec2, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

int vector_norm(double *norm, const vector_t vec);

/* ------------------------------------------------------------------------- */

int vector_unit(vector_t *unit, const vector_t vec);

/* ------------------------------------------------------------------------- */

int vector_dot(double *dot_product, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

int vector3_cross(vector_t *cross, const vector_t vec1, const vector_t vec2);

/* ------------------------------------------------------------------------- */

bool vector_equal(const vector_t vec1, const vector_t vec2, const double tolerance);

/* ------------------------------------------------------------------------- */

#endif