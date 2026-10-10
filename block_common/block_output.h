#ifndef BLOCK_OUTPUT_H
#define BLOCK_OUTPUT_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <stdbool.h>

#include "utils/error_handling.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

typedef enum {
    OUTPUT_EPH = 0,
    OUTPUT_TLE = 1,
    OUTPUT_TIM = 2
} output_t;

typedef struct {
    int n_TLE;
    char **aTLE;
    int n_EPH;
    char **aEPH;
    int n_TIM;
    char **aTIM;
} block_output_t;

// ----------------------------------------------------------------------------

err_t block_output_push(block_output_t *item, const output_t typ,
                        const char *path);
err_t block_output_create_json(cJSON **json, const block_output_t *item);
err_t block_output_read_json(block_output_t *item, const cJSON *json);

err_t block_output_reset(block_output_t *item);

#endif // BLOCK_OUTPUT_H
