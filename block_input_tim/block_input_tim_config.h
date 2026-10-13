#ifndef BLOCK_INPUT_TIM_CONFIG_H
#define BLOCK_INPUT_TIM_CONFIG_H
/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>

#include "external/cJSON.h"
#include "utils/error_handling.h"

// Module data structures -----------------------------------------------------

typedef struct {
    char *start;
    char *end;
    double stepsize_seconds;
    bool is_single;
} input_tim_config_t;

// ----------------------------------------------------------------------------

err_t input_tim_config_read_json(input_tim_config_t *item, const cJSON *json);

#endif // BLOCK_INPUT_TIM_CONFIG_H
