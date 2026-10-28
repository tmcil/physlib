#include "block_input_epo_config.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t input_epo_config_read_json(input_epo_config_t *item, const cJSON *json) {
    // Local variables
    cJSON *aux = NULL;

    if (item == NULL) return ERR("Cannot read JSON to NULL pointer");

    aux = cJSON_GetObjectItemCaseSensitive(json, "start");
    if (!cJSON_IsString(aux)) return ERR("Failed to read JSON: start");
    item->start = strdup(aux->valuestring);

    aux = cJSON_GetObjectItemCaseSensitive(json, "end");
    if (!cJSON_IsString(aux)) return ERR("Failed to read JSON: end");
    item->end = strdup(aux->valuestring);

    aux = cJSON_GetObjectItemCaseSensitive(json, "stepsizeSeconds");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read JSON: stepsizeSeconds");
    item->stepsize_seconds = aux->valuedouble;

    return NO_ERROR;
}
