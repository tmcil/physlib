#include "tle.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t tle_read_json(tle_t *item, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    cJSON *aux = NULL;

    if (json == NULL) return ERR("NULL cJSON argument passed");

    err = tle_reset(item);
    if (err) return ERR("Failed to reset tle_t");

    aux = cJSON_GetObjectItemCaseSensitive(json, "satelliteId");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read: satelliteId");
    item->satellite_id = aux->valueint;

    aux = cJSON_GetObjectItemCaseSensitive(json, "objectName");
    if (!cJSON_IsString(aux)) return ERR("Failed to read: objectName");
    item->object_name = strdup(aux->valuestring);
    if (item->object_name==NULL) return ERR("Failed to allocate objectName");

    aux = cJSON_GetObjectItemCaseSensitive(json, "line1");
    if (!cJSON_IsString(aux)) return ERR("Failed to read: line1");
    strncpy(item->line_1, aux->valuestring, sizeof(item->line_1) - 1);
    item->line_1[sizeof(item->line_1) - 1] = '\0';

    aux = cJSON_GetObjectItemCaseSensitive(json, "line2");
    if (!cJSON_IsString(aux)) return ERR("Failed to read: line2");
    strncpy(item->line_2, aux->valuestring, sizeof(item->line_2) - 1);
    item->line_2[sizeof(item->line_2) - 1] = '\0';

    cJSON_Delete(aux);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t tle_create_json(cJSON **json, const tle_t *item) {
    // Local variables
    cJSON *aux = NULL;

    if (item == NULL) return ERR("Bad item pointer argument passed");
    if (tle_isreset(item)) return WAR("Cannot write an empty tle");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("TLE");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    aux = cJSON_CreateNumber(item->satellite_id);
    if (aux == NULL) return ERR("Failed to create json: satellite_id");
    cJSON_AddItemToObject(*json, "satelliteId", aux);

    aux = cJSON_CreateString(item->object_name);
    if (aux == NULL) return ERR("Failed to create json: object_name");
    cJSON_AddItemToObject(*json, "objectName", aux);

    aux = cJSON_CreateString(item->line_1);
    if (aux == NULL) return ERR("Failed to create json: line_1");
    cJSON_AddItemToObject(*json, "line1", aux);

    aux = cJSON_CreateString(item->line_2);
    if (aux == NULL) return ERR("Failed to create json: line_2");
    cJSON_AddItemToObject(*json, "line2", aux);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t tle_init(tle_t *item, const int satellite_id, const char *object_name,
               const char *line_1, const char *line_2) {
    // Local variables
    err_t err = NO_ERROR;

    if (item==NULL) return ERR("Attempted to init a null pointer");

    err = tle_reset(item);
    if (err) return ERR("Failed to reset existing TLE");

    item->satellite_id = satellite_id;

    item->object_name = malloc(strlen(object_name) + 1);
    if (item->object_name == NULL) return ERR("Failed to allocate memory");
    strcpy(item->object_name, object_name);

    strncpy(item->line_1, line_1, sizeof(item->line_1) - 1);
    item->line_1[sizeof(item->line_1) - 1] = '\0';

    strncpy(item->line_2, line_2, sizeof(item->line_2) - 1);
    item->line_2[sizeof(item->line_2) - 1] = '\0';

    return NO_ERROR;
    }

// ----------------------------------------------------------------------------
err_t tle_reset(tle_t *item) {
    if (item == NULL) return ERR("Attempted to reset a NULL pointer");
    free(item->object_name);
    item->object_name = NULL;
    item->line_1[0] = '\0';
    item->line_2[0] = '\0';
    item->satellite_id = -1;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
bool tle_isreset(const tle_t *item){
    return (item->satellite_id==-1 && item->object_name==NULL &&
            item->line_1[0]=='\0' && item->line_2[0]=='\0');
}
