#ifndef TIM_H
#define TIM_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "utils/error_handling.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

typedef enum {
    EMPTY = -1,
    JD = 0,
    J2000 = 1,
    MJD = 2,
    DAYFRACDAY = 3
} jd_format_t;

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
} datetime_t;

// Date format is consistent with SOFA astrodynamics library. Method member is
// purely for the developer, the two components always sum to the same value.
typedef struct {
    double date1;
    double date2;
    jd_format_t format;
} epoch_t;

typedef struct {
    epoch_t start;
    epoch_t end;
    size_t n_epochs;
    epoch_t *epochs;
} tim_t;

// ----------------------------------------------------------------------------
err_t tim_read_json(tim_t *item, bool *is_utc, const cJSON *json);
err_t tim_create_json(cJSON **json, const tim_t *item, const bool is_utc);
err_t tim_init(tim_t *item, const epoch_t start, const epoch_t end,
               const double stepsize_seconds);
err_t tim_reset(tim_t *item);
bool tim_isreset(const tim_t *item);


epoch_t epoch_add(const epoch_t epoch1, const epoch_t epoch2);
epoch_t epoch_addsec(const epoch_t epoch, const double seconds);
epoch_t epoch_addday(const epoch_t epoch, const double days);
epoch_t epoch_sub(const epoch_t epoch1, const epoch_t epoch2);
double epoch_days(const epoch_t epoch);
double epoch_seconds(const epoch_t epoch);
bool epoch_eq(const epoch_t epoch1, const epoch_t epoch2);
bool epoch_gt(const epoch_t epoch1, const epoch_t epoch2);
err_t epoch_to_iso8601(char **iso, const epoch_t *epoch, const bool is_utc);
err_t epoch_from_iso8601(epoch_t *item, bool *is_utc, const char* iso,
                         const jd_format_t format);
err_t epoch_change_format(epoch_t *item, const jd_format_t format);
err_t epoch_reset(epoch_t *item);
bool epoch_isreset(const epoch_t item);

#endif // TIM_H
