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

// ----------------------------------------------------------------------------
epoch_t epoch_addsec(const epoch_t epoch, const double seconds) {
    // Local variables
    epoch_t res = epoch;

    res.date2 += seconds / 86400.0;
    return res;
}

// ----------------------------------------------------------------------------
epoch_t epoch_addday(const epoch_t epoch, const double days) {
    // Local variables
    epoch_t res = epoch;

    res.date2 += days;
    return res;
}

// ----------------------------------------------------------------------------
// return must always be checked !isreset by the caller.
epoch_t epoch_add(const epoch_t epoch1, const epoch_t epoch2) {
    // Local variables
    epoch_t sum;

    epoch_reset(&sum);

    // return the reset epoch of there is an error.
    if (epoch_isreset(epoch1) || epoch_isreset(epoch2)) return sum;

    sum.format = JD;
    sum.date1 = epoch1.date1+epoch1.date2+epoch2.date1+epoch2.date2;
    epoch_change_format(&sum, J2000);

    return sum;
}

// ----------------------------------------------------------------------------
// return must always be checked !isreset by the caller.
epoch_t epoch_sub(const epoch_t epoch1, const epoch_t epoch2) {
    // Local variables
    epoch_t sum;

    epoch_reset(&sum);

    // return the reset epoch of there is an error.
    if (epoch_isreset(epoch1) || epoch_isreset(epoch2)) return sum;

    sum.format = JD;
    sum.date1 = (epoch1.date1+epoch1.date2)-(epoch2.date1+epoch2.date2);
    epoch_change_format(&sum, J2000);

    return sum;
}

// ----------------------------------------------------------------------------
double epoch_days(const epoch_t epoch) {
    return (epoch.date1+epoch.date2);
}

// ----------------------------------------------------------------------------
double epoch_seconds(const epoch_t epoch) {
    return (epoch.date1+epoch.date2) * 86400.0;
}

// ----------------------------------------------------------------------------
bool epoch_eq(const epoch_t epoch1, const epoch_t epoch2) {
    return fabs(epoch1.date1+epoch1.date2 - epoch2.date1+epoch2.date2) < 1e-12;
}

// ----------------------------------------------------------------------------
bool epoch_gt(const epoch_t epoch1, const epoch_t epoch2) {
    return (epoch1.date1+epoch1.date2 > epoch2.date1+epoch2.date2+1e-12);
}

// ----------------------------------------------------------------------------
err_t epoch_to_iso8601(char **iso, const epoch_t *epoch, const bool is_utc) {
    // Local variables
    err_t err = NO_ERROR;
    datetime_t datetime = {0};
    epoch_t aux_epoch = *epoch;
    int J, a, n, A, i, b, j, l;
    double fracday, total_seconds;
    char iso_buf[25];

    if (iso == NULL) return ERR("NULL pointer to ISO string");
    if (epoch_isreset(aux_epoch)) return ERR("Empty epoch passed to function");

    err = epoch_change_format(&aux_epoch, JD);
    if (err) return ERR("Failed to modify epoch to JD format");

    if (aux_epoch.date1 <= 0.0) {
        return ERR("Negative julian day");
    }

    J = (int)floor(aux_epoch.date1 + 0.5);
    fracday =  aux_epoch.date1 + 0.5 - J;

    a = J + 68569;
    n = 4 * a / 146097;
    A = a - (146097 * n + 3) / 4;
    i = 4000 * (A + 1) / 1461001;
    b = A - 1461 * i / 4 + 31;
    j = 80 * b / 2447;

    datetime.day = b - 2447 * j / 80;

    l = j / 11;
    datetime.month = j + 2 - 12 * l;
    datetime.year = 100 * (n - 49) + i + l;

    total_seconds = fracday * 86400.0;

    datetime.hour = (int)(total_seconds / 3600.0);
    total_seconds -= datetime.hour * 3600.0;

    datetime.minute = (int)(total_seconds / 60.0);
    datetime.second = total_seconds - datetime.minute * 60.0;

    // Check for valid date
    // Allow 61 seconds for leap seconds, in some applications
    if (datetime.month < 1 || datetime.month > 12 ||
        datetime.day < 1 || datetime.day > 31 ||
        datetime.hour < 0 || datetime.hour > 23 ||
        datetime.minute < 0 || datetime.minute > 59 ||
        datetime.second < 0 || datetime.second > 61.0) {
        return ERR("Invalid date value(s): %d%d%d %02d:%02d:%f",
                   datetime.year, datetime.month, datetime.day, datetime.hour,
                   datetime.minute, datetime.second);
    }

    if (datetime.year < 1900 || datetime.year > 2100) {
        return WAR("Date outwith valid period for datetime -> JD conversion "
                       "(1900 < datetime < 2100).");
    }

    if (is_utc) {
        snprintf(iso_buf, 25, "%04d-%02d-%02dT%02d:%02d:%06.3fZ",
                 datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
    } else {
        snprintf(iso_buf, 24, "%04d-%02d-%02dT%02d:%02d:%06.3f",
                 datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
    }

    *iso = strdup(iso_buf);
    if (*iso == NULL) return ERR("Failed to allocate memory");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t epoch_from_iso8601(epoch_t *item, bool *is_utc, const char* iso,
                         const jd_format_t format) {
    // Local variables
    err_t err = NO_ERROR;
    int iso_len = 0;
    datetime_t datetime = {0};
    epoch_t aux_epoch = {0};

    if (iso==NULL) return ERR("ISO datetime is NULL");

    iso_len = (int)strlen(iso);
    if (iso_len == 23) {
        *is_utc = false;
    } else if (iso_len == 24) {
        *is_utc = true;
    } else {
        ERR("Expected format: YYYY-M-DDTHH:MM:SS.sss(Z)");
        return ERR("Invalid ISO8601 date: %s", iso);
    }

    // Check seperators
    if (iso[4] != '-' || iso[7] != '-' || iso[10] != 'T' || iso[13] != ':' ||
        iso[16] != ':') {
        ERR("Expected format: YYYY-M-DDTHH:MM:SS.sss(Z)");
        return ERR("Invalid ISO8601 date: %s", iso);
    }

    // Parse date
    datetime.year = atoi(iso+0);
    datetime.month = atoi(iso+5);
    datetime.day = atoi(iso+8);
    datetime.hour = atoi(iso+11);
    datetime.minute = atoi(iso+14);
    datetime.second = atof(iso+17);

    // Check for valid date
    // Allow 61 seconds for leap seconds, in some applications
    if (datetime.month < 1 || datetime.month > 12 ||
        datetime.day < 1 || datetime.day > 31 ||
        datetime.hour < 0 || datetime.hour > 23 ||
        datetime.minute < 0 || datetime.minute > 59 ||
        datetime.second < 0 || datetime.second > 61.0) {
        ERR("Expected format: YYYY-M-DDTHH:MM:SS.sss(Z)");
        return ERR("Invalid date value(s): %s", iso);
    }

    if (datetime.year < 1900 || datetime.year > 2100) {
        return WAR("Date outwith valid period for datetime -> JD conversion "
                       "(1900 < datetime < 2100).");
    }

    // Convert to julian date
    epoch_reset(&aux_epoch);
    aux_epoch.format = JD;
    aux_epoch.date1 = (
        367 * datetime.year - (int)(7*(datetime.year + 
        (int)(((double)(datetime.month)+9)/12))/4) +
        (int)(275*datetime.month/9) + datetime.day + 1721013.5 +
        ((datetime.second/60 + datetime.minute)/60 + datetime.hour)/24
    );

    err = epoch_change_format(&aux_epoch, format);
    if (err) return ERR("Failed to update format of epoch");

    *item = aux_epoch;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t epoch_change_format(epoch_t *item, const jd_format_t format) {
    // Local variables
    long double sum = 0.0;
    long double new1 = 0.0;
    long double new2 = 0.0;

    sum = (long double)item->date1 + (long double)item->date2;

    switch (format) {
        case JD:
            new1 = sum;
            new2 = 0.0;
            break;
        case J2000:
            new1 = 2451545.0;
            new2 = sum - new1;
            break;
        case MJD:
            new1 = 2400000.5;
            new2 = sum - new1;
            break;
        case DAYFRACDAY:
            new1 = floor(sum*2.0)/2.0;
            new2 = sum - new1;
            break;
        default:
            return ERR("Invalid format requested");
    }

    item->date1 = (double)new1;
    item->date2 = (double)new2;
    item->format = format;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t epoch_reset(epoch_t *item) {
    item->date1 = 0.0;
    item->date2 = 0.0;
    item->format = EMPTY;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
bool epoch_isreset(const epoch_t item) {
    return (item.date1 < 1e-12 && item.date2 < 1e-12 && item.format == EMPTY);
}
