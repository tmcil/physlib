
#include "epoch.h"


/* ------------------------------------------------------------------------- */

void epoch_zero(epoch_t *epo) {
    epo->date1 = 0.0;
    epo->date2 = 0.0;
    epo->format = J2000;
    epo->scale = UTC;
    return;
}

/* ------------------------------------------------------------------------- */

bool epoch_iszerod(const epoch_t epo) {
    return (epo.date1 < 1e-12 && epo.date2 < 1e-12 && epo.format == JD &&
            epo.scale == UTC);
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_days(const double delta) {
    // Local variables
    bool negative = false;                                               //     Delta is negative
    double local_delta = 0.0;                                            //     Copy of input delta
    double days = 0, hours = 0, minutes = 0, seconds = 0;                //     Time components
    double fracday = 0.0, frachour = 0.0, fracmin = 0.0, fracsec = 0.0;  //     Fractional times
    double millis = 0.0;                                                 //     Milliseconds of delta

    negative = delta < 0.0;
    local_delta = fabs(delta);

    fracday = modf(local_delta, &days);
    hours = fracday * 24.0;
    frachour = modf(hours, &hours);
    minutes = frachour * 60.0;
    fracmin = modf(minutes, &minutes);
    seconds = fracmin * 60.0;
    fracsec = modf(seconds, &seconds);
    millis = fracsec * 1000.0;

    return (epoch_delta_t) {
        .negative = negative,
        .days = (unsigned int) days,
        .hours = (unsigned int) hours,
        .minutes = (unsigned int) minutes,
        .seconds = (unsigned int) seconds,
        .millis = millis
    };
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_hours(const double hours) {
    double days = hours / 24.0;
    return delta_days(days);
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_minutes(const double minutes) {
    double days = minutes / 1440.0;
    return delta_days(days);
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_seconds(const double seconds) {
    double days = seconds / 86400.0;
    return delta_days(days);
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_millis(const double millis) {
    double days = millis / (86400.0 * 1000.0);
    return delta_days(days);
}

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_from_epochs(const epoch_t epo1, const epoch_t epo2) {
    // Local variables
    epoch_t aux1 = {0}, aux2 = {0};  //     Auxiliary epochs
    double days = 0.0;               //     Total delta in days

    // Switch to j2000 and subtract second component, avoids precision loss
    epoch_copy(&aux1, epo1);
    epoch_copy(&aux2, epo2);
    epoch_switch_format(&aux1, J2000);
    epoch_switch_format(&aux2, J2000);
    days = aux2.date2 - aux1.date2;
    return delta_days(days);
}

/* ------------------------------------------------------------------------- */

double delta_to_days(const epoch_delta_t delta) {
    int sign = (delta.negative) ? -1 : 1;
    return (sign * (delta.days + delta.hours/24.0 + delta.minutes/1440.0 +
                    delta.seconds/86400.0 + delta.millis/(86400.0*1000.0)));
}

/* ------------------------------------------------------------------------- */

err_t epoch_from_delta(epoch_t *epo, const epoch_t ref, const epoch_delta_t delta) {
    double days = delta_to_days(delta);
    epo->scale = ref.scale;
    epo->format = ref.format;
    epo->date1 = ref.date1;
    epo->date2 = ref.date2 + days;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t epoch_copy(epoch_t *dst, const epoch_t src) {
    if (dst == NULL) return ERR("Destination pointer is null");
    memcpy(dst, &src, sizeof(epoch_t));
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t epoch_now(epoch_t *epo) {
    // Local variables
    time_t now = time(NULL);  //     Current time (unix)
    struct tm current_time;   //     Date-time object

    if (now == (time_t)-1 || gmtime_r(&now, &current_time) == NULL) {
        return -1;
    }

    int year = current_time.tm_year + 1900;
    int month = current_time.tm_mon + 1;

    epo->scale = UTC;
    epoch_switch_format(epo, JD);

    epo->date2 = 0.0;
    epo->date1 = (
        367 * year
        - (int)(7 * (year + (int)((month + 9) / 12)) / 4)
        + (int)(275 * month / 9)
        + current_time.tm_mday
        + 1721013.5
        + ((current_time.tm_sec / 60.0 + current_time.tm_min) / 60.0
           + current_time.tm_hour) / 24.0
    );

    epoch_switch_format(epo, J2000);
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t epoch_switch_format(epoch_t *epo, epoch_format_t format) {
    // Local variables
    double rem = 0.0;               //     Reminder w.r.t new1, to be added to new2
    double new1 = 0.0, new2 = 0.0;  //     New date components

    if (epo->format == format) return NO_ERROR;

    // Must avoid computing (epo->date1 + epo->date2) to avoid precision loss
    switch (format) {
        case MJD:
            new1 = 2400000.5;
            break;
        case J2000:
            new1 = 2451545.0;
            break;
        case JD:
            // Unavoidable here, since by definition the user is requesting the
            // combined value
            new1 = epo->date1 + epo->date2;
            break;
        default:
            return ERR("Unsupported epoch date format");
    }

    rem = epo->date1 - new1;
    new2 = epo->date2 + rem;

    epo->format = format;
    epo->date1 = new1;
    epo->date2 = new2;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool epoch_less(const epoch_t epo1, const epoch_t epo2) {
    // Local variables
    epoch_t aux1 = {0}, aux2 = {0};  //     Auxiliary epochs

    epoch_copy(&aux1, epo1);
    epoch_copy(&aux2, epo2);
    epoch_switch_format(&aux1, J2000);
    epoch_switch_format(&aux2, J2000);

    return (aux1.date2 < aux2.date2);
}

/* ------------------------------------------------------------------------- */

bool epoch_greater(const epoch_t epo1, const epoch_t epo2) {
    return epoch_less(epo2, epo1);
}

/* ------------------------------------------------------------------------- */

bool epoch_equal(const epoch_t epo1, const epoch_t epo2, const epoch_delta_t tolerance) {
    // Local variables
    epoch_t aux1 = {0}, aux2 = {0};  //     Auxiliary epochs

    epoch_copy(&aux1, epo1);
    epoch_copy(&aux2, epo2);
    epoch_switch_format(&aux1, J2000);
    epoch_switch_format(&aux2, J2000);

    return fabs(aux1.date2 - aux2.date2) < delta_to_days(tolerance);
}

/* ------------------------------------------------------------------------- */

/* Checks an epoch exists within a given interval                            */
/*  |<---------------------------- acceptable --------------------------->|  */
/*  |       lower                                             upper       |  */
/*  |<--tol-->|-------------------------------------------------|<--tol-->|  */
bool epoch_between(const epoch_t epo, const epoch_t lower, const epoch_t upper, const epoch_delta_t tolerance) {
    // Local variables
    epoch_t lower_bound = {0}, upper_bound = {0};                           //     Bounds to check
    epoch_delta_t lower_tol = delta_days(-fabs(delta_to_days(tolerance)));  //     Lower tolerance
    epoch_delta_t upper_tol = delta_days(fabs(delta_to_days(tolerance)));   //     Upper tolerance

    epoch_from_delta(&lower_bound, lower, lower_tol);
    epoch_from_delta(&upper_bound, upper, upper_tol);

    if (epoch_less(epo, lower_bound)) return false;
    if (epoch_greater(epo, upper_bound)) return false;
    return true;
}

/* ------------------------------------------------------------------------- */

err_t epoch_from_iso8601(epoch_t *epo, const char *iso) {
    // Local variables
    int iso_len = 0;                     //     length of input iso string
    int year, month, day, hour, minute;  //     Datetime elements
    double second;                       //     Seconds


    if (iso==NULL) return ERR("ISO string pointer is null");

    iso_len = (int)strlen(iso);
    if (iso_len == 23) {
        epo->scale = UT1;
    } else if (iso_len == 24) {
        epo->scale = UTC;
    } else {
        return ERR("ISO string has invalid length");
    }

    // Check seperators
    if (iso[4] != '-' || iso[7] != '-' || iso[10] != 'T' || iso[13] != ':' ||
        iso[16] != ':') {
        return ERR("ISO date separators are invalid");
    }

    // Parse date
    year = atoi(iso+0);
    month = atoi(iso+5);
    day = atoi(iso+8);
    hour = atoi(iso+11);
    minute = atoi(iso+14);
    second = atof(iso+17);

    // Check for valid date
    // Allow 61 seconds for leap seconds, in some applications
    if (month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 61.0) {
        return ERR("ISO date fields are out of range");
    }

    if (year < 1900 || year > 2100) {
        return ERR("ISO year is out of range");
    }

    // Convert to julian date
    if (epoch_switch_format(epo, JD)) return ERR("Failed to convert epoch to Julian date");
    epo->date2 = 0.0;
    epo->date1 = (
        367 * year - (int)(7*(year + 
        (int)(((double)(month)+9)/12))/4) +
        (int)(275*month/9) + day + 1721013.5 +
        ((second/60 + minute)/60 + hour)/24
    );

    if (epoch_switch_format(epo, J2000)) return ERR("Failed to convert epoch to J2000 format");

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t epoch_to_iso8601(char* iso, size_t iso_len, const epoch_t epoch) {
    // Local variables
    int err = 0;                         //     Error code
    epoch_t aux_epoch = epoch;           //     Auxiliary epoch, copy of input
    int J, a, n, A, i, b, j, l;          //     Intermediate parameters for JD calc
    int year, month, day, hour, minute;  //     Datetime elements
    double second;                       //     Seconds
    double fracday, total_seconds;       //     Fractional day elements

    if (iso == NULL) return ERR("Output ISO buffer is null");
    if (iso_len < 25) return ERR("Output ISO buffer is too small");

    err = epoch_switch_format(&aux_epoch, JD);
    if (err) return ERR("Failed to convert epoch format");

    if (aux_epoch.date1 <= 0.0) {
        return ERR("Julian date must be positive");
    }

    J = (int)floor(aux_epoch.date1 + 0.5);
    fracday =  aux_epoch.date1 + 0.5 - J;

    a = J + 68569;
    n = 4 * a / 146097;
    A = a - (146097 * n + 3) / 4;
    i = 4000 * (A + 1) / 1461001;
    b = A - 1461 * i / 4 + 31;
    j = 80 * b / 2447;

    day = b - 2447 * j / 80;

    l = j / 11;
    month = j + 2 - 12 * l;
    year = 100 * (n - 49) + i + l;

    total_seconds = fracday * 86400.0;

    hour = (int)(total_seconds / 3600.0);
    total_seconds -= hour * 3600.0;

    minute = (int)(total_seconds / 60.0);
    second = total_seconds - minute * 60.0;

    // Check for valid date
    // Allow 61 seconds for leap seconds, in some applications
    if (month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 61.0) {
        return ERR("ISO date fields are out of range");
    }

    if (year < 1900 || year > 2100) {
        return ERR("ISO year is out of range");
    }

    if (epoch.scale == UTC) {
        snprintf(iso, iso_len, "%04d-%02d-%02dT%02d:%02d:%06.3fZ",
                 year, month, day, hour, minute, second);
    } else {
        snprintf(iso, iso_len, "%04d-%02d-%02dT%02d:%02d:%06.3f",
                 year, month, day, hour, minute, second);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t interval_init(interval_t *interval, const size_t capacity) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (interval == NULL) return ERR("Interval pointer is null");
    if (capacity <= 0) return ERR("Interval capacity must be positive");
    if (interval_is_allocated(interval)) interval_free(interval);

    interval->capacity = capacity;
    interval->epochs = malloc(capacity * sizeof(epoch_t));
    if (interval->epochs == NULL) {
        interval->capacity = 0;
        return ERR("Failed to allocate interval storage");
    }
    for (i = 0; i < capacity; i++) {
        epoch_zero(&interval->epochs[i]);
    }

    interval->n_epochs = 0;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool interval_is_allocated(const interval_t *interval) {
    if (interval == NULL) return false;
    return (interval->epochs != NULL && interval->capacity != 0);
}

/* ------------------------------------------------------------------------- */

void interval_free(interval_t *interval) {
    if (interval_is_allocated(interval)) {
        free(interval->epochs);
        interval->epochs = NULL;
        interval->capacity = 0;
        interval->n_epochs = 0;
    }
}

/* ------------------------------------------------------------------------- */

err_t interval_push(interval_t *interval, const epoch_t epoch) {
    // Local variables
    epoch_t *aux = NULL;                       //     Pointer to interval after reallocation
    size_t new_capacity = interval->capacity;  //     Capacity after reallocation

    if (!interval_is_allocated(interval)) return ERR("Interval is not allocated");

    if (interval->n_epochs > 0) {
        // If adding to an existing list, check the epoch is after the previous
        if (epoch_less(epoch, interval->epochs[interval->n_epochs-1])) return ERR("Epoch precedes last interval epoch");
    }

    if (interval->n_epochs >= interval->capacity) {
        new_capacity = (interval->capacity == 0) ? 1 : 2 * interval->capacity;
        aux = realloc(interval->epochs, new_capacity * sizeof(epoch_t));
        if (aux == NULL) return ERR("Failed to realloc interval memory");
        interval->epochs = aux;
        interval->capacity = new_capacity;
    }

    interval->epochs[interval->n_epochs] = epoch;
    interval->n_epochs += 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t interval_linear(interval_t *interval, const epoch_t start, const epoch_t end,
                    const epoch_delta_t stepsize) {
    // Local variables
    size_t i = 0;                  //     Iterator
    size_t n_epochs = 0;           //     Number of epochs in the interval
    double span_days = 0.0;        //     Total span of the interval, in days
    double stepsize_days = 0.0;    //     Stepsize of the interval, in days
    epoch_t previous_epoch = {0};  //     Last epoch pushed to the interval

    if (epoch_iszerod(start)) return ERR("Start time is zerod");
    if (epoch_iszerod(end)) return ERR("End time is zerod");
    if (stepsize.negative == true) return ERR("Negative stepsize");
    if (epoch_less(end, start) || epoch_equal(start, end, delta_millis(1))) {
        // In this case, return an interval with a single element == start
        if (interval_init(interval, 1)) return ERR("Failed to init interval");
        if (interval_push(interval, start)) return ERR("Failed to push epoch");
        return NO_ERROR;
    }

    epoch_delta_t span = delta_from_epochs(start, end);
    span_days = delta_to_days(span);
    stepsize_days = delta_to_days(stepsize);
    if (stepsize_days*86400*1000 <= 1) return ERR("Stepsize too small");  // milliseconds check
    n_epochs = (size_t) ceil(span_days/stepsize_days + 1);

    if (interval_init(interval, n_epochs)) return ERR("Failed to init interval");
    interval->epochs[0] = start;
    previous_epoch = start;
    for (i = 1; i < n_epochs; i++) {
        if (i == n_epochs - 1) {
            interval->epochs[i] = end;
        } else {
            epoch_from_delta(&interval->epochs[i], previous_epoch, stepsize);
        }
        previous_epoch = interval->epochs[i];
    }
    
    interval->n_epochs = n_epochs;

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */