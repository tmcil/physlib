#ifndef BLOCK_INPUT_TLE_CONFIG_H
#define BLOCK_INPUT_TLE_CONFIG_H
/**
 * Author: Thomas McIlwraith
 */

#include "external/cJSON.h"
#include "core/error_handling.h"

// Module data structures -----------------------------------------------------

typedef struct {
    char line_1[70];
    char line_2[70];
} input_tle_config_t;

// ----------------------------------------------------------------------------

err_t input_tle_config_read_json(input_tle_config_t *item, const cJSON *json);

#endif // BLOCK_INPUT_TLE_CONFIG_H
