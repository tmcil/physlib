#include "tim.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t tim_read_json(tim_t *item, bool *is_utc, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    cJSON *aux = NULL;
    cJSON *aux_arr = NULL;
    cJSON *aux_arr_el = NULL;
    epoch_t aux_epoch = {0};
    size_t i = 0;
    bool seen_utc = false;

    if (json == NULL) return ERR("NULL cJSON argument passed");
    err = tim_reset(item);
    if (err) return ERR("Failed to reset tim_t");

    aux = cJSON_GetObjectItemCaseSensitive(json, "start");
    if (!cJSON_IsString(aux)) return ERR("Failed to read: start");
    err = epoch_from_iso8601(&aux_epoch, &seen_utc, aux->valuestring, J2000);
    if (err) return ERR("Failed to parse start epoch: %s", aux->valuestring);
    *is_utc = seen_utc;
    item->start = aux_epoch;

    aux = cJSON_GetObjectItemCaseSensitive(json, "end");
    if (!cJSON_IsString(aux)) return ERR("Failed to read: end");
    err = epoch_from_iso8601(&aux_epoch, &seen_utc, aux->valuestring, J2000);
    if (err) return ERR("Failed to parse end epoch: %s", aux->valuestring);
    if (seen_utc != *is_utc) return ERR("Mismatched UTC in epochs");
    item->end = aux_epoch;

    aux_arr = cJSON_GetObjectItemCaseSensitive(json, "epochs");
    if (!cJSON_IsArray(aux_arr)) return ERR("Failed to read epochs array in JSON");
    item->n_epochs = (size_t)cJSON_GetArraySize(aux_arr);

    item->epochs = malloc(item->n_epochs * sizeof(epoch_t));
    for (i = 0; i < item->n_epochs; i++) {
        aux_arr_el = cJSON_GetArrayItem(aux_arr, i);
        aux = cJSON_GetObjectItemCaseSensitive(aux_arr_el, "epoch");
        if (!cJSON_IsString(aux)) return ERR("Failed to read epoch string(%d)", i);

        err = epoch_from_iso8601(&aux_epoch, &seen_utc, aux->valuestring, J2000);
        if (err) return ERR("Failed to parse epoch[%d]: %s", i, aux->valuestring);
        if (seen_utc != *is_utc) return ERR("Mismatched UTC in epochs");
        item->epochs[i] = aux_epoch;

    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t tim_create_json(cJSON **json, const tim_t *item, const bool is_utc){
    // Local variables
    err_t err = NO_ERROR;
    cJSON *aux = NULL;
    cJSON *aux_array = NULL;
    cJSON *aux_array_el = NULL;
    char *iso_buf = NULL;
    size_t i = 0;

    if (item == NULL) return ERR("Bad item pointer argument passed");
    if (tim_isreset(item)) return WAR("Cannot write an empty ephemeris");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("TIM");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    err = epoch_to_iso8601(&iso_buf, &item->start, is_utc);
    if (err) return ERR("Failed to convert epoch to ISO string");
    aux = cJSON_CreateString(iso_buf);
    if (aux == NULL) return ERR("Failed to create json: start");
    cJSON_AddItemToObject(*json, "start", aux);

    err = epoch_to_iso8601(&iso_buf, &item->end, is_utc);
    if (err) return ERR("Failed to convert epoch to ISO string");
    aux = cJSON_CreateString(iso_buf);
    if (aux == NULL) return ERR("Failed to create json: end");
    cJSON_AddItemToObject(*json, "end", aux);

    aux_array = cJSON_AddArrayToObject(*json, "epochs");
    if (aux_array == NULL) return ERR("Failed to add epoch array to JSON object");
    for (i = 0; i < item->n_epochs; i++) {
        err = epoch_to_iso8601(&iso_buf, &item->epochs[i], is_utc);
        if (err) return ERR("Failed to convert epoch to ISO string");

        aux_array_el = cJSON_CreateObject();
        if (aux_array_el == NULL) return ERR("Failed to create object for array");
        cJSON_AddItemToArray(aux_array, aux_array_el);
    
        aux = cJSON_CreateString(iso_buf);
        if (aux == NULL) return ERR ("Failed to create iso string JSON");
        cJSON_AddItemToObject(aux_array_el, "epoch", aux);
    }

    free(iso_buf);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t tim_init(tim_t *item, const epoch_t start, const epoch_t end,
               const double stepsize_seconds) {
    // Local variables
    err_t err = NO_ERROR;
    size_t n_epochs = 0;
    epoch_t span = {0};
    size_t i = 0;

    epoch_reset(&span);

    if (item == NULL) return ERR("Cannot generate epochs for NULL pointer");
    if (epoch_isreset(start)) return ERR("start epoch is reset");
    if (epoch_isreset(end)) return ERR("end epoch is reset");
    if (stepsize_seconds <= 0.0) return ERR("stepsize must be positive");

    err = tim_reset(item);
    if (err) return ERR("Failed to reset tim type");

    if (epoch_gt(start, end)) return ERR("start time is after end time");

    span = epoch_sub(end, start);
    if (epoch_isreset(span)) return ERR("Failed to subtract epochs");

    n_epochs = (int)ceil((epoch_seconds(span)/stepsize_seconds + 1));
    if (n_epochs <= 0) return ERR("Invalid number of epochs computed: %s", n_epochs);

    item->epochs = malloc(n_epochs * sizeof(epoch_t));
    if (item->epochs == NULL) return ERR("Failed to allocate memory");

    for (i = 0; i < n_epochs; i++) {
        if (i == n_epochs -1) {
            item->epochs[i] = end;
        } else {
            item->epochs[i] = epoch_addsec(start, i * stepsize_seconds);
        }
    }

    item->start = item->epochs[0];
    item->end = item->epochs[n_epochs-1];
    item->n_epochs = n_epochs;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
bool tim_isreset(const tim_t *item) {
    return (epoch_isreset(item->start) && epoch_isreset(item->end) &&
            item->n_epochs == 0 && item->epochs == NULL);
}

// ----------------------------------------------------------------------------
err_t tim_reset(tim_t *item) {
    if (item==NULL) return ERR("Attempted to reset a NULL pointer");
    epoch_reset(&item->start);
    epoch_reset(&item->end);
    if (item->epochs != NULL) free(item->epochs);
    item->epochs = NULL;
    return NO_ERROR;
}
