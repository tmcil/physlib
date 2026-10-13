#include "block_sgp4_config.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t block_sgp4_config_read(sgp4_config_t *item, const cJSON *json) {
    // Local variables
    cJSON *aux = NULL;

    if (item == NULL) return ERR("Cannot read JSON to NULL pointer");

    aux = cJSON_GetObjectItemCaseSensitive(json, "wgsModel");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read wgsModel");
    item->wgs_model = (wgs_model_t)aux->valueint;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
// err_t input_tle_config_read_json(input_tle_config_t *item, const cJSON *json) {
//     // Local variables
//     cJSON *aux = NULL;

//     if (item == NULL) return ERR("Cannot read JSON to NULL pointer");

//     aux = cJSON_GetObjectItemCaseSensitive(json, "objectName");
//     if (!cJSON_IsString(aux)) return ERR("Failed to read objectName");
//     item->object_name = strdup(aux->valuestring);

//     aux = cJSON_GetObjectItemCaseSensitive(json, "satelliteId");
//     if (!cJSON_IsNumber(aux)) return ERR("Failed to read satelliteId");
//     item->satellite_id = aux->valueint;

//     aux = cJSON_GetObjectItemCaseSensitive(json, "line1");
//     if (!cJSON_IsString(aux)) return ERR("Failed to read: line1");
//     strncpy(item->line_1, aux->valuestring, sizeof(item->line_1) - 1);
//     item->line_1[sizeof(item->line_1) - 1] = '\0';

//     aux = cJSON_GetObjectItemCaseSensitive(json, "line2");
//     if (!cJSON_IsString(aux)) return ERR("Failed to read: line2");
//     strncpy(item->line_2, aux->valuestring, sizeof(item->line_2) - 1);
//     item->line_2[sizeof(item->line_2) - 1] = '\0';

//     return NO_ERROR;
// }
