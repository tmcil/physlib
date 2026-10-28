#ifndef BLOCK_SGP4_CONFIG_H
#define BLOCK_SGP4_CONFIG_H
/**
 * Author: Thomas McIlwraith
 */

#include "external/cJSON.h"
#include "core/error_handling.h"

typedef enum {
    WGS72_OLD = 1,
    WGS72 = 2,
    WGS84 = 3
} wgs_model_t;

typedef struct {
    wgs_model_t wgs_model;
} sgp4_config_t;

/* ------------------------------------------------------------------------- */

err_t block_sgp4_config_read(sgp4_config_t *item, const cJSON *json);

/* ------------------------------------------------------------------------- */

#endif // BLOCK_SGP4_CONFIG_H
