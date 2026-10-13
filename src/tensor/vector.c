#include "vector.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t vector_reset(vector_t *vec) {
    epoch_reset(&vec->epoch);
    vec->vec[0] = 0.0;
    vec->vec[1] = 0.0;
    vec->vec[2] = 0.0;
    vec->vec[3] = 0.0;
    vec->vec[4] = 0.0;
    vec->vec[5] = 0.0;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t vector_print(const vector_t vec) {
    char *iso_buf = NULL;
    epoch_to_iso8601(&iso_buf, &vec.epoch, true);
    MSG("EPOCH : %s", iso_buf);
    MSG("VECTOR: %f %f %f %f %f %f",
        vec.vec[0], vec.vec[1], vec.vec[2], vec.vec[3], vec.vec[4], vec.vec[5]);
    return NO_ERROR;
}
