#ifndef ORBIT_FRAME_H
#define ORBIT_FRAME_H

#include "core/error_handling.h"
#include "core/matrix.h"
#include "core/epoch.h"
#include "core/cartsv.h"
#include "core/linalg.h"
#include "core/constants.h"
#include "external/sofa/sofa.h"


typedef struct {
    planetary_constants_t center_body;
    epoch_t tt;
    epoch_t utc;
    epoch_t ut1;
    double x_pole;  // Radians
    double y_pole;  // Radians
    double tio_locator;  // Radians
    double earth_rotation_angle;  // Radians
    double dX_cip;
    double dY_cip;
    int sofa_c2i_method;
} orbit_conversion_params_t;


/* ------------------------------------------------------------------------- */

err_t orbit_frame_convert(cartsv_t *out, const orbit_frame_t out_frame,
                          const cartsv_t in, const orbit_frame_t in_frame);

/* ------------------------------------------------------------------------- */

err_t tirs_2_itrs(cartsv_t *itrs, const cartsv_t *tirs, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

err_t cirs_2_tirs(cartsv_t *tirs, const cartsv_t *cirs, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

err_t gcrs_2_cirs(cartsv_t *cirs, const cartsv_t *gcrs, const orbit_conversion_params_t params);

/* ------------------------------------------------------------------------- */

#endif
