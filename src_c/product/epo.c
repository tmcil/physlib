#include "epo.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t epo_read_json(interval_t *item, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    cJSON *aux = NULL;
    cJSON *aux_arr = NULL;
    cJSON *aux_arr_el = NULL;
    epoch_t aux_epoch = {0};
    size_t i = 0;
    size_t arr_len = 0;

    if (json == NULL) return ERR("NULL cJSON argument passed");
    err = epo_reset(item);
    if (err) return ERR("Failed to reset interval_t");

    aux_arr = cJSON_GetObjectItemCaseSensitive(json, "values");
    if (!cJSON_IsArray(aux_arr)) return ERR("Failed to read epochs array in JSON");

    arr_len = cJSON_GetArraySize(aux_arr);
    if (arr_len <= 0) return ERR("Non-positive array length in EPO");

    err = interval_init(item, arr_len);
    if (err) return ERR("Failed to initialise interval");

    for (i = 0; i < arr_len; i++) {
        aux_arr_el = cJSON_GetArrayItem(aux_arr, i);
        aux = cJSON_GetObjectItemCaseSensitive(aux_arr_el, "epoch");
        if (!cJSON_IsString(aux)) return ERR("Failed to read epoch string(%d)", i);

        err = epoch_from_iso8601(&aux_epoch, aux->valuestring);
        if (err) return ERR("Failed to parse epoch[%d]: %s", i, aux->valuestring);

        err = interval_push(item, aux_epoch);
        if (err) return ERR("Failed to add epoch to interval");
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t epo_create_json(cJSON **json, const interval_t *item){
    // Local variables
    err_t err = NO_ERROR;
    size_t i = 0;
    char iso_buf[32];
    cJSON *aux = NULL;
    cJSON *aux_array = NULL;
    cJSON *aux_array_el = NULL;

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("EPO");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    err = epoch_to_iso8601(iso_buf, 32, item->epochs[0]);
    if (err) return ERR("Failed to convert epoch to ISO string");
    aux = cJSON_CreateString(iso_buf);
    if (aux == NULL) return ERR("Failed to create json: start");
    cJSON_AddItemToObject(*json, "start", aux);

    err = epoch_to_iso8601(iso_buf, 32, item->epochs[item->n_epochs-1]);
    if (err) return ERR("Failed to convert epoch to ISO string");
    aux = cJSON_CreateString(iso_buf);
    if (aux == NULL) return ERR("Failed to create json: end");
    cJSON_AddItemToObject(*json, "end", aux);

    aux_array = cJSON_AddArrayToObject(*json, "values");
    if (aux_array == NULL) return ERR("Failed to add epoch array to JSON object");
    for (i = 0; i < item->n_epochs; i++) {
        err = epoch_to_iso8601(iso_buf, 32, item->epochs[i]);
        if (err) return ERR("Failed to convert epoch to ISO string");

        aux_array_el = cJSON_CreateObject();
        if (aux_array_el == NULL) return ERR("Failed to create object for array");
        cJSON_AddItemToArray(aux_array, aux_array_el);
    
        aux = cJSON_CreateString(iso_buf);
        if (aux == NULL) return ERR ("Failed to create iso string JSON");
        cJSON_AddItemToObject(aux_array_el, "epoch", aux);
    }

    return NO_ERROR;
}
