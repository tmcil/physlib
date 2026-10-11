#include "vector.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t vector_done(vector_t *vec) {
    epoch_reset(&vec->epoch);
    vec->vec[0] = 0.0;
    vec->vec[1] = 0.0;
    vec->vec[2] = 0.0;
    vec->vec[3] = 0.0;
    vec->vec[4] = 0.0;
    vec->vec[5] = 0.0;
    return NO_ERROR;
}
