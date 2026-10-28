#ifndef EPO_H
#define EPO_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "core/error_handling.h"
#include "external/cJSON.h"
#include "core/epoch.h"


/* ------------------------------------------------------------------------- */

err_t epo_read_json(interval_t *item, const cJSON *json);

/* ------------------------------------------------------------------------- */

err_t epo_create_json(cJSON **json, const interval_t *item);

/* ------------------------------------------------------------------------- */

#endif // EPO_H
