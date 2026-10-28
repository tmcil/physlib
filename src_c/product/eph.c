
#include "eph.h"


/* ------------------------------------------------------------------------- */

err_t eph_create_json(cJSON **json, const cartsv_array_t *item) {
    // Local variables
    cJSON *aux = NULL;           //     Auxiliary JSON field storage
    cJSON *vec_array = NULL;     //     Values array
    cJSON *vec_array_el = NULL;  //     Values array[i]
    char iso_buf[32];            //     Buffer for ISO8601 timestamp
    size_t j = 0;                //     Iterator

    if (item == NULL) return ERR("Bad item pointer argument passed");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("EPH");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    vec_array = cJSON_AddArrayToObject(*json, "values");
    if (vec_array == NULL) return ERR("Failed to create json: state_vectors");

    for (size_t i = 0; i < item->n_statevectors; i++) {
        vec_array_el = cJSON_CreateObject();

        epoch_to_iso8601(iso_buf, 32, item->statevectors[i].epoch);
        aux = cJSON_CreateString(iso_buf);
        if (aux == NULL) return ERR("Failed to create JSON for epoch: %s", iso_buf);
        cJSON_AddItemToObject(vec_array_el, "epoch", aux);

        aux = cJSON_CreateArray();
        for (j = 0; j < item->statevectors[i].r.dims; j++) {
            cJSON_AddItemToArray(aux, cJSON_CreateNumber(item->statevectors[i].r.values[j]));
        }
        cJSON_AddItemToObject(vec_array_el, "r", aux);

        aux = cJSON_CreateArray();
        for (j = 0; j < item->statevectors[i].v.dims; j++) {
            cJSON_AddItemToArray(aux, cJSON_CreateNumber(item->statevectors[i].v.values[j]));
        }
        cJSON_AddItemToObject(vec_array_el, "v", aux);

        aux = cJSON_CreateArray();
        for (j = 0; j < item->statevectors[i].cov.rows * item->statevectors[i].cov.cols; j++) {
            cJSON_AddItemToArray(aux, cJSON_CreateNumber(item->statevectors[i].cov.values[j]));
        }
        cJSON_AddItemToObject(vec_array_el, "cov", aux);

        cJSON_AddItemToArray(vec_array, vec_array_el);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t eph_read_json(cartsv_array_t *item, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;        //     Error code
    cJSON *aux = NULL;           //     Auxiliary JSON field storage
    cJSON *vec_array = NULL;     //     Values array
    cJSON *vec_array_el = NULL;  //     Values array[i]
    int values_len = 0;          //     Length of values array
    cartsv_t statevector = {0};  // MEM Statevector buffers

    if (json == NULL) return ERR("NULL cJSON argument passed");

    vec_array = cJSON_GetObjectItemCaseSensitive(json, "values");
    if (!cJSON_IsArray(vec_array)) return ERR("Failed to read: values");

    values_len = cJSON_GetArraySize(vec_array);
    if (values_len <=1 ) return ERR("Empty values array in EPH");

    err = cartsv_array_init(item, values_len);
    if (err) return ERR("Failed to initiliase trajectory from EPH");

    statevector.frame = ORBIT_FRAME_J2000;
    vector_init(&statevector.r, 3);
    vector_init(&statevector.v, 3);
    matrix_init(&statevector.cov, 6, 6);

    for (int i = 0; i < values_len; i++) {

        vec_array_el = cJSON_GetArrayItem(vec_array, i);
        if (!cJSON_IsObject(vec_array_el)) return ERR("Failed to read: values[%d]", i);
        
        aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "epoch");
        if (!cJSON_IsString(aux)) return ERR("Failed to read epoch[%d]", i);
        err = epoch_from_iso8601(&statevector.epoch, aux->valuestring);
        if (err) return ERR("Failed to convert ISO string to epoch: %s", aux->valuestring);

        aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "r");
        if (!cJSON_IsArray(aux)) return ERR("Failed to read r[%d]", i);
        for (int j = 0; j < cJSON_GetArraySize(aux); j++) {
            statevector.r.values[j] = cJSON_GetArrayItem(aux, j)->valuedouble;
        }

        aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "v");
        if (!cJSON_IsArray(aux)) return ERR("Failed to read v[%d]", i);
        for (int j = 0; j < cJSON_GetArraySize(aux); j++) {
            statevector.v.values[j] = cJSON_GetArrayItem(aux, j)->valuedouble;
        }

        aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "cov");
        if (!cJSON_IsArray(aux)) return ERR("Failed to read cov[%d]", i);
        for (int j = 0; j < cJSON_GetArraySize(aux); j++) {
            statevector.cov.values[j] = cJSON_GetArrayItem(aux, j)->valuedouble;
        }

        err = cartsv_array_push(item, statevector);
        if (err) return ERR("Failed to push statevector to trajectory");
    }

    cartsv_free(&statevector);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */