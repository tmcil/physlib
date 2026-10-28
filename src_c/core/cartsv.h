#ifndef CARTSV_H
#define CARTSV_H

#include <stddef.h>

#include "error_handling.h"
#include "epoch.h"
#include "matrix.h"
#include "orbit_frame.h"
#include "vector.h"


typedef struct cartsv {
    orbit_frame_t frame;
    epoch_t epoch;
    vector_t r;
    vector_t v;
    matrix_t cov;
} cartsv_t;

typedef struct {
    cartsv_t *statevectors;
    size_t n_statevectors;
    size_t capacity;
} cartsv_array_t;


/* ------------------------------------------------------------------------- */

err_t cartsv_init(cartsv_t *sv, const int dims, const orbit_frame_t frame);

/* ------------------------------------------------------------------------- */

bool cartsv_is_allocated(const cartsv_t *sv);

/* ------------------------------------------------------------------------- */

void cartsv_free(cartsv_t *sv);

/* ------------------------------------------------------------------------- */

err_t cartsv_copy(cartsv_t *dst, const cartsv_t src);

/* ------------------------------------------------------------------------- */

err_t cartsv_rotate(cartsv_t *out, const orbit_frame_t out_frame,
                  const cartsv_t in, const orbit_frame_t in_frame);

/* ------------------------------------------------------------------------- */

err_t cartsv_array_init(cartsv_array_t *trajectory, const size_t capacity);

/* ------------------------------------------------------------------------- */

bool cartsv_array_is_allocated(const cartsv_array_t *trajectory);

/* ------------------------------------------------------------------------- */

void cartsv_array_free(cartsv_array_t *trajectory);

/* ------------------------------------------------------------------------- */

err_t cartsv_array_push(cartsv_array_t *trajectory, const cartsv_t statevector);

/* ------------------------------------------------------------------------- */
#endif
