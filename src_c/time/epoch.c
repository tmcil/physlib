#include "epoch.h"

// Module Variables -----------------------------------------------------------


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
