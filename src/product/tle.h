#ifndef TLE_H
#define TLE_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>
#include <stdlib.h>

#include "utils/error_handling.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

typedef struct {
    char line_1[70];
    char line_2[70];
} tle_t;

// ----------------------------------------------------------------------------

err_t tle_create_json(cJSON **json, const tle_t *item);
err_t tle_read_json(tle_t *item, const cJSON *json);

err_t tle_init(tle_t *item, const char *line_1, const char *line_2);

err_t tle_reset(tle_t *item);
bool tle_isreset(const tle_t *item);

#endif // EPH_H
