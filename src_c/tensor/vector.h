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
err_t vector_reset(vector_t *vec);
err_t vector_print(const vector_t vec);

#endif // VECTOR_H
