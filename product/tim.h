#ifndef TIM_H
#define TIM_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "utils/error_handling.h"
#include "external/cJSON.h"
#include "time/epoch.h"

// Module data structures -----------------------------------------------------

typedef struct {
    epoch_t start;
    epoch_t end;
    size_t n_epochs;
    epoch_t *epochs;
} tim_t;

// ----------------------------------------------------------------------------
err_t tim_read_json(tim_t *item, bool *is_utc, const cJSON *json);
err_t tim_create_json(cJSON **json, const tim_t *item, const bool is_utc);
err_t tim_init(tim_t *item, const epoch_t start, const epoch_t end,
               const double stepsize_seconds);
err_t tim_reset(tim_t *item);
bool tim_isreset(const tim_t *item);

#endif // TIM_H
