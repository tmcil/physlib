#ifndef BLOCK_INPUT_EPO_CONFIG_H
#define BLOCK_INPUT_EPO_CONFIG_H
/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>

#include "external/cJSON.h"
#include "core/error_handling.h"

// Module data structures -----------------------------------------------------

typedef struct {
    char *start;
    char *end;
    double stepsize_seconds;
} input_epo_config_t;

// ----------------------------------------------------------------------------

err_t input_epo_config_read_json(input_epo_config_t *item, const cJSON *json);

#endif // BLOCK_INPUT_EPO_CONFIG_H
