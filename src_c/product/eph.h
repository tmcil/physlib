#ifndef EPH_H
#define EPH_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>
#include <stdlib.h>

#include "core/error_handling.h"
#include "external/cJSON.h"
#include "core/cartsv.h"

/* ------------------------------------------------------------------------- */

err_t eph_create_json(cJSON **json, const cartsv_array_t *item);

/* ------------------------------------------------------------------------- */

err_t eph_read_json(cartsv_array_t *item, const cJSON *json);

/* ------------------------------------------------------------------------- */

#endif // EPH_H
