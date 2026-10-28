#ifndef EPOCH_H
#define EPOCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "error_handling.h"


typedef enum {
    JD,
    J2000,
    MJD
} epoch_format_t;

typedef enum {
    UTC,
    UT1,
    TAI
} epoch_scale_t;

typedef struct {
    const bool negative;
    const unsigned int days;
    const unsigned int hours;
    const unsigned int minutes;
    const unsigned int seconds;
    const double millis;
} epoch_delta_t;

typedef struct {
    double date1;
    double date2;
    epoch_format_t format;
    epoch_scale_t scale;
} epoch_t;

typedef struct {
    epoch_t *epochs;
    size_t n_epochs;
    size_t capacity;
} interval_t;


/* ------------------------------------------------------------------------- */

void epoch_zero(epoch_t *epo);

/* ------------------------------------------------------------------------- */

bool epoch_iszerod(const epoch_t epo);

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_days(const double delta);

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_hours(const double hours);

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_minutes(const double minutes);

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_seconds(const double seconds);

/* ------------------------------------------------------------------------- */

epoch_delta_t delta_millis(const double millis);

/* ------------------------------------------------------------------------- */

/**
 * @brief returns epo2 - epo1
 */
epoch_delta_t delta_from_epochs(const epoch_t epo1, const epoch_t epo2);

/* ------------------------------------------------------------------------- */

double delta_to_days(const epoch_delta_t delta);

/* ------------------------------------------------------------------------- */

int epoch_from_delta(epoch_t *epo, const epoch_t ref, const epoch_delta_t delta);

/* ------------------------------------------------------------------------- */

int epoch_copy(epoch_t *dst, const epoch_t src);

/* ------------------------------------------------------------------------- */

int epoch_now(epoch_t *epo);

/* ------------------------------------------------------------------------- */

int epoch_switch_format(epoch_t *epo, epoch_format_t format);

/* ------------------------------------------------------------------------- */

bool epoch_less(const epoch_t epo1, const epoch_t epo2);

/* ------------------------------------------------------------------------- */

bool epoch_greater(const epoch_t epo1, const epoch_t epo2);

/* ------------------------------------------------------------------------- */

bool epoch_equal(const epoch_t epo1, const epoch_t epo2, const epoch_delta_t tolerance);

/* ------------------------------------------------------------------------- */

bool epoch_between(const epoch_t epo, const epoch_t lower, const epoch_t upper, const epoch_delta_t tolerance);

/* ------------------------------------------------------------------------- */

int epoch_from_iso8601(epoch_t *epo, const char *iso);

/* ------------------------------------------------------------------------- */

int epoch_to_iso8601(char *iso, size_t iso_len, const epoch_t epoch);

/* ------------------------------------------------------------------------- */

int interval_init(interval_t *interval, const size_t capacity);

/* ------------------------------------------------------------------------- */

bool interval_is_allocated(const interval_t *interval);

/* ------------------------------------------------------------------------- */

void interval_free(interval_t *interval);

/* ------------------------------------------------------------------------- */

int interval_push(interval_t *interval, const epoch_t epoch);

/* ------------------------------------------------------------------------- */

int interval_linear(interval_t *interval, const epoch_t start, const epoch_t end,
                    const epoch_delta_t stepsize);

/* ------------------------------------------------------------------------- */

#endif
