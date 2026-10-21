#include "eph.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t eph_create_json(cJSON **json, const eph_t *item) {
    // Local variables
    cJSON *aux = NULL;
    cJSON *vec_array = NULL;
    cJSON *cov_array = NULL;
    cJSON *vec_array_el = NULL;
    cJSON *cov_array_el = NULL;
    char *iso_buf;

    if (item == NULL) return ERR("Bad item pointer argument passed");
    if (eph_isreset(item)) return WAR("Cannot write an empty ephemeris");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString("EPH");
    if (aux == NULL) return ERR("Failed to create json: type");
    cJSON_AddItemToObject(*json, "type", aux);

    aux = cJSON_CreateNumber(item->center_body);
    if (aux == NULL) return ERR("Failed to create json: center_body");
    cJSON_AddItemToObject(*json, "centerBody", aux);

    aux = cJSON_CreateNumber(item->reference_frame);
    if (aux == NULL) return ERR("Failed to create json: reference_frame");
    cJSON_AddItemToObject(*json, "referenceFrame", aux);

    aux = cJSON_CreateNumber(item->meters_scale_factor);
    if (aux == NULL) return ERR("Failed to create json: meters_scale_factor");
    cJSON_AddItemToObject(*json, "metersScaleFactor", aux);

    aux = cJSON_CreateNumber(item->n_datapoints);
    if (aux == NULL) return ERR("Failed to create json: n_datapoints");
    cJSON_AddItemToObject(*json, "nDatapoints", aux);

    aux = cJSON_CreateBool(item->has_state_vectors);
    if (aux == NULL) return ERR("Failed to create json: has_state_vectors");
    cJSON_AddItemToObject(*json, "hasStateVectors", aux);

    aux = cJSON_CreateBool(item->has_covariance_matrices);
    if (aux == NULL) return ERR("Failed to create json: has_covariance_matrices");
    cJSON_AddItemToObject(*json, "hasCovarianceMatrices", aux);

    if (item->has_state_vectors) {
        vec_array = cJSON_AddArrayToObject(*json, "data");
        if (vec_array == NULL) return ERR("Failed to create json: state_vectors");

        for (int i = 0; i < item->n_datapoints; i++) {
            vec_array_el = cJSON_CreateObject();

            epoch_to_iso8601(&iso_buf, &item->state_vectors[i].epoch, true);
            aux = cJSON_CreateString(iso_buf);
            if (aux == NULL) return ERR("Failed to create JSON for epoch: %s", iso_buf);
            cJSON_AddItemToObject(vec_array_el, "epoch", aux);

            aux = cJSON_CreateArray();
            for (int j = 0; j < 6; j++) {
                cJSON_AddItemToArray(aux, cJSON_CreateNumber(item->state_vectors[i].vec[j]));
            }
            cJSON_AddItemToObject(vec_array_el, "vec", aux);

            cJSON_AddItemToArray(vec_array, vec_array_el);
        }
    }

    if (item->has_covariance_matrices) {
        cov_array = cJSON_AddArrayToObject(*json, "covarianceMatrices");
        if (cov_array == NULL) return ERR("Failed to create json: covariance_matrices");

        for (int i = 0; i < item->n_datapoints; i++) {
            cov_array_el = cJSON_CreateObject();
            aux = cJSON_CreateNumber(item->covariance_matrices[i].epoch);
            if (aux == NULL) return ERR("Failed to create json: covariance_matrices[%d].epoch", i);
            cJSON_AddItemToObject(cov_array_el, "epoch", aux);

            aux = cJSON_CreateArray();
            for (int j = 0; j < 21; j++) {
                cJSON_AddItemToArray(aux, cJSON_CreateNumber(item->covariance_matrices[i].cov[j]));
            }
            cJSON_AddItemToObject(cov_array_el, "cov", aux);

            cJSON_AddItemToArray(cov_array, cov_array_el);
        }
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t eph_read_json(eph_t *item, const cJSON *json) {
    // Local variables
    cJSON *aux = NULL;
    cJSON *vec_array = NULL;
    cJSON *cov_array = NULL;
    cJSON *vec_array_el = NULL;
    cJSON *cov_array_el = NULL;
    vector_t aux_vec = {0};
    cov_t aux_cov = {0};
    err_t err;
    epoch_t epoch = {0};
    bool is_utc = false;

    if (json == NULL) return ERR("NULL cJSON argument passed");

    err = eph_reset(item);
    if (err) return ERR("Failed to reset eph_t");

    aux = cJSON_GetObjectItemCaseSensitive(json, "centerBody");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read: centerBody");
    item->center_body = aux->valueint;

    aux = cJSON_GetObjectItemCaseSensitive(json, "referenceFrame");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read: referenceFrame");
    item->reference_frame = aux->valueint;

    aux = cJSON_GetObjectItemCaseSensitive(json, "metersScaleFactor");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read: metersScaleFactor");
    item->meters_scale_factor = aux->valuedouble;

    aux = cJSON_GetObjectItemCaseSensitive(json, "nDatapoints");
    if (!cJSON_IsNumber(aux)) return ERR("Failed to read: nDatapoints");
    item->n_datapoints = aux->valueint;

    aux = cJSON_GetObjectItemCaseSensitive(json, "hasStateVectors");
    if (!cJSON_IsBool(aux)) return ERR("Failed to read: hasStateVectors");
    item->has_state_vectors = cJSON_IsTrue(aux);

    aux = cJSON_GetObjectItemCaseSensitive(json, "hasCovarianceMatrices");
    if (!cJSON_IsBool(aux)) return ERR("Failed to read: hasCovarianceMatrices");
    item->has_covariance_matrices = cJSON_IsTrue(aux);

    if (item->has_state_vectors) {
        item->state_vectors = malloc(item->n_datapoints * sizeof(vector_t));
        if (item->state_vectors == NULL) return ERR("Failed to allocate: state_vectors");

        vec_array = cJSON_GetObjectItemCaseSensitive(json, "stateVectors");
        if (!cJSON_IsArray(vec_array)) return ERR("Failed to read: stateVectors");

        if (item->n_datapoints != cJSON_GetArraySize(vec_array)) {
            WAR("Mismatched datapoint count in EPH file. Using nDatapoints count");
        }

        for (int i = 0; i < item->n_datapoints; i++) {
            vec_array_el = cJSON_GetArrayItem(vec_array, i);
            if (!cJSON_IsObject(vec_array_el)) return ERR("Failed to read: statevector[%d]", i);
            
            aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "epoch");
            if (!cJSON_IsString(aux)) return ERR("Failed to read epoch[%d]", i);
            err = epoch_from_iso8601(&epoch, &is_utc, aux->valuestring, J2000);
            if (err) return ERR("Failed to convert ISO string to epoch: %s", aux->valuestring);
            aux_vec.epoch = epoch;

            aux = cJSON_GetObjectItemCaseSensitive(vec_array_el, "vec");
            if (!cJSON_IsArray(aux)) return ERR("Failed to read vec[%d]", i);
            for (int j = 0; j < cJSON_GetArraySize(aux); j++) {
                aux_vec.vec[j] = cJSON_GetArrayItem(aux, j)->valuedouble;
            }

            item->state_vectors[i] = aux_vec;
        }
    }

    if (item->has_covariance_matrices) {
        item->covariance_matrices = malloc(item->n_datapoints * sizeof(cov_t));
        if (item->covariance_matrices == NULL) return ERR("Failed to allocate: covariance_matrices");

        cov_array = cJSON_GetObjectItemCaseSensitive(json, "covarianceMatrices");
        if (!cJSON_IsArray(cov_array)) return ERR("Failed to read: covarianceMatrices");

        if (item->n_datapoints != cJSON_GetArraySize(cov_array)) {
            WAR("Mismatched datapoint count in EPH file. Using nDatapoints count");
        }

        for (int i = 0; i < item->n_datapoints; i++) {
            cov_array_el = cJSON_GetArrayItem(cov_array, i);
            if (!cJSON_IsObject(cov_array_el)) return ERR("Failed to read: covariance[%d]", i);
            
            aux = cJSON_GetObjectItemCaseSensitive(cov_array_el, "epoch");
            if (!cJSON_IsNumber(aux)) return ERR("Failed to read epoch[%d]", i);
            aux_cov.epoch = aux->valuedouble;

            aux = cJSON_GetObjectItemCaseSensitive(cov_array_el, "cov");
            if (!cJSON_IsArray(aux)) return ERR("Failed to read cov[%d]", i);
            for (int j = 0; j < cJSON_GetArraySize(aux); j++) {
                aux_cov.cov[j] = cJSON_GetArrayItem(aux, j)->valuedouble;
            }

            item->covariance_matrices[i] = aux_cov;
        }
    }

    cJSON_Delete(aux);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t eph_push(eph_t *item, const vector_t *vec, const cov_t *cov,
              const bool add_vec, const bool add_cov) {
    // Local variables
    vector_t *aux_vec = NULL;
    cov_t *aux_cov = NULL;

    if (item == NULL) return ERR("NULL eph_t argument passed");
    if (vec == NULL && add_vec) return ERR("NULL vector_t passed to eph_addvec");
    if (cov == NULL && add_cov) return ERR("NULL cov_t passed to eph_addvec");
    if (!add_cov && !add_vec) return WAR("No add option selected");
    if ((add_cov && add_vec) && (item->has_covariance_matrices ^
                                 item->has_state_vectors)) {
        return ERR("attempted operation which would result in mismatched vec/cov data lengths");
    }
    if ((add_cov ^ add_vec) && (item->has_covariance_matrices &&
                                 item->has_state_vectors)) {
        return ERR("attempted operation which would result in mismatched vec/cov data lengths");
    }

    if (add_vec) {
        aux_vec = realloc(item->state_vectors,
                          (item->n_datapoints + 1) * sizeof(*aux_vec));
        if (aux_vec == NULL) return ERR("State vector allocation failed");
        item->state_vectors = aux_vec;
        item->state_vectors[item->n_datapoints] = *vec;
        item->has_state_vectors = true;
    }

    if (add_cov) {
        aux_cov = realloc(item->covariance_matrices,
                          (item->n_datapoints + 1) * sizeof(*aux_cov));
        if (aux_cov == NULL) return ERR("Covariance allocation failed");
        item->covariance_matrices = aux_cov;
        item->covariance_matrices[item->n_datapoints] = *cov;
        item->has_covariance_matrices = true;
    }

    if (add_vec || add_cov ) item->n_datapoints++;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t eph_init(eph_t *item, const int center_body, const int reference_frame,
               const double meters_scale_factor){
    // Local variables
    err_t err = NO_ERROR;
    
    if (item == NULL) return ERR("Attempted to init a null pointer");

    err = eph_reset(item);
    if (err) return ERR("Failed to reset existing ephemeris");

    item->center_body = center_body;
    item->reference_frame = reference_frame;
    item->meters_scale_factor = meters_scale_factor;
    item->n_datapoints = 0;
    item->has_covariance_matrices = false;
    item->has_state_vectors = false;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t eph_reset(eph_t *item) {
    if (item == NULL) return ERR("Attempted to reset a NULL pointer");
    free(item->state_vectors);
    item->state_vectors = NULL;
    free(item->covariance_matrices);
    item->covariance_matrices = NULL;
    item->center_body = -1;
    item->reference_frame = -1;
    item->meters_scale_factor = -1;
    item->n_datapoints = -1;
    item->has_state_vectors = false;
    item->has_covariance_matrices = false;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
bool eph_isreset(const eph_t *item) {
    return (item->center_body==-1 &&
            item->reference_frame==-1 && item->n_datapoints==-1 &&
            !item->has_state_vectors && !item->has_covariance_matrices);
}
