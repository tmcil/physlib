#ifndef VECTOR_H
#define VECTOR_H

/**
 * Author: Thomas McIlwraith
 */

#include "utils/error_handling.h"
#include "time/epoch.h"

// Module data structures -----------------------------------------------------

typedef struct {
    epoch_t epoch;
    double vec[6];
} vector_t;

// ----------------------------------------------------------------------------
err_t vector_done(vector_t *vec);

#endif // VECTOR_H
