#include "block_output.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t block_output_create_json(cJSON **json, const block_output_t *item) {
    // Local variables
    cJSON *aux = NULL;
    cJSON *el = NULL;

    if (item == NULL) return ERR("Bad item pointer argument passed");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("BLOCK_OUTPUT");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    // Add the aTLE array
    aux = cJSON_AddArrayToObject(*json, "aTLE");
    if (aux == NULL) return ERR("Failed to add aTLE array to JSON object");
    for (int i = 0; i<item->n_TLE; i++) {
        el = cJSON_CreateString(item->aTLE[i]);
        if (el == NULL) return ERR("Failed to create TLE[%d]", i);
        cJSON_AddItemToArray(aux, el);
    }

    // Add the aEPH array
    aux = cJSON_AddArrayToObject(*json, "aEPH");
    if (aux == NULL) return ERR("Failed to add aEPH array to JSON object");
    for (int i = 0; i<item->n_EPH; i++) {
        el = cJSON_CreateString(item->aEPH[i]);
        if (el == NULL) return ERR("Failed to create EPH[%d]", i);
        cJSON_AddItemToArray(aux, el);
    }

    // Add the aTIM array
    aux = cJSON_AddArrayToObject(*json, "aTIM");
    if (aux == NULL) return ERR("Failed to add aTIM array to JSON object");
    for (int i = 0; i<item->n_TIM; i++) {
        el = cJSON_CreateString(item->aTIM[i]);
        if (el == NULL) return ERR("Failed to create TIM[%d]", i);
        cJSON_AddItemToArray(aux, el);
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t block_output_push(block_output_t *item, const output_t typ,
                        const char *path) {
    // Local variables
    char ***target_arr = NULL;
    int *target_len = NULL;

    if (item == NULL) return ERR("Cannot push new element to NULL pointer");

    // select the correct member of the struct 
    switch (typ) {
        case OUTPUT_EPH:
            target_arr = &item->aEPH;
            target_len = &item->n_EPH;
            break;
        case OUTPUT_TLE:
            target_arr = &item->aTLE;
            target_len = &item->n_TLE;
            break;
        case OUTPUT_TIM:
            target_arr = &item->aTIM;
            target_len = &item->n_TIM;
            break;
        default:
            target_arr = NULL;
            target_len = NULL;
            return ERR("Unsupported output type: %d", typ);
    }

    // Resize the array
    *target_arr = realloc(*target_arr, (*target_len + 1) * sizeof(char *));
    if (*target_arr == NULL) return ERR("Failed to reallocate array pointers");

    // Allocate memory for this entry
    (*target_arr)[*target_len] = strdup(path);
    if ((*target_arr)[*target_len] == NULL) return ERR("Failed to allocate memory");

    // Increase the counter
    *target_len = *target_len + 1;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t block_output_read_json(block_output_t *item, const cJSON *json) {
    // Local variables
    cJSON *aTLE = NULL;
    cJSON *aEPH = NULL;
    cJSON *aTIM = NULL;

    if (item == NULL) return ERR("Cannot read JSON to NULL pointer");

    // Assign the variable length arrays
    aTLE = cJSON_GetObjectItemCaseSensitive(json, "aTLE");
    if (cJSON_IsArray(aTLE) == false) return ERR("TLE is missing from output file");
    aEPH = cJSON_GetObjectItemCaseSensitive(json, "aEPH");
    if (cJSON_IsArray(aEPH) == false) return ERR("EPH is missing from output file");
    aTIM = cJSON_GetObjectItemCaseSensitive(json, "aTIM");
    if (cJSON_IsArray(aTIM) == false) return ERR("TIM is missing from output file");

    // Assign all the static values
    item->n_TLE = cJSON_GetArraySize(aTLE);
    item->n_EPH = cJSON_GetArraySize(aEPH);
    item->n_TIM = cJSON_GetArraySize(aTIM);

    // Allocate the memory for the arrays
    item->aTLE = malloc(item->n_TLE * sizeof(char *));
    item->aEPH = malloc(item->n_EPH * sizeof(char *));
    item->aTIM = malloc(item->n_TIM * sizeof(char *));
    if (item->aTLE == NULL || item->aEPH == NULL || item->aTIM == NULL) {
        return ERR("Failed to allocate memory for application output arrays");
    }

    // Load all the arrays
    for (int i = 0; i < item->n_TLE; i++) {
        item->aTLE[i] = strdup(cJSON_GetArrayItem(aTLE, i)->valuestring);
        if (item->aTLE[i] == NULL) {
            return ERR("Failed to allocate memory for TLE");
        }
    }
    for (int i = 0; i < item->n_EPH; i++) {
        item->aEPH[i] = strdup(cJSON_GetArrayItem(aEPH, i)->valuestring);
        if (item->aEPH[i] == NULL) {
            return ERR("Failed to allocate memory for EPH");
        }
    }
    for (int i = 0; i < item->n_TIM; i++) {
        item->aTIM[i] = strdup(cJSON_GetArrayItem(aTIM, i)->valuestring);
        if (item->aTIM[i] == NULL) {
            return ERR("Failed to allocate memory for TIM");
        }
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t block_output_reset(block_output_t *item) {
    if (item==NULL) return ERR("Attempted to reset a NULL pointer");

    for (int i = 0; i < item->n_TLE; i++) {
        free(item->aTLE[i]);
        item->aTLE[i] = NULL;
    }

    for (int i = 0; i < item->n_EPH; i++) {
        free(item->aEPH[i]);
        item->aEPH[i] = NULL;
    }

    for (int i = 0; i < item->n_TIM; i++) {
        free(item->aTIM[i]);
        item->aTIM[i] = NULL;
    }

    item->n_TLE = 0;
    item->n_EPH = 0;
    item->n_TIM = 0;
    return NO_ERROR;
}
