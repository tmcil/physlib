#ifndef ORBIT_FRAME_H
#define ORBIT_FRAME_H

#include "core/error_handling.h"
#include "core/matrix.h"
#include "core/epoch.h"
#include "core/cartsv.h"


typedef struct {
    epoch_t utc;
    epoch_t ut1;
    epoch_t tt;
    epoch_t tai;
    double x_pole;
    double y_pole;
    double tio_locator;
    double earth_rotation_angle;
    double dX_cip;
    double dY_cip;
} orbit_conversion_params_t;


/* ------------------------------------------------------------------------- */

err_t orbit_frame_convert(cartsv_t *out, const orbit_frame_t out_frame,
                          const cartsv_t in, const orbit_frame_t in_frame);

/* ------------------------------------------------------------------------- */

err_t tirs_2_itrs(matrix_t *pos, matrix_t *vel, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

err_t cirs_2_tirs(matrix_t *pos, matrix_t *vel, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

err_t gcrs_2_cirs(matrix_t *pos, matrix_t *vel, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

#endif
