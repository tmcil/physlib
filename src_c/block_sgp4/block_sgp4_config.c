
#include "block_sgp4_config.h"


/* ------------------------------------------------------------------------- */

err_t block_sgp4_config_read(sgp4_config_t *item, const cJSON *json) {
    // Local variables
    cJSON *aux = NULL; //     Auxiliary JSON field storage

    if (item == NULL) return ERR("Cannot read JSON to NULL pointer");

    aux = cJSON_GetObjectItemCaseSensitive(json, "wgsModel");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read wgsModel");
    item->wgs_model = (wgs_model_t)aux->valueint;

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */
