
#include "linalg.h"


/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_x(matrix_t *rotmat, const double rad) {
    // Local variables
    err_t status = NO_ERROR;    //     Error code
    bool failed = false;        //     Flag to indicate failure of set operation
    const double c = cos(rad);  //     Cosine of the rotation angle
    const double s = sin(rad);  //     Sine of the rotation angle

    status = matrix_init(rotmat, 3, 3);
    if (status != NO_ERROR) return ERR("Failed to initialize rotation matrix");

    if (matrix_set(rotmat, 0, 0, 1.0)) failed = true;
    if (matrix_set(rotmat, 0, 1, 0.0)) failed = true;
    if (matrix_set(rotmat, 0, 2, 0.0)) failed = true;
    if (matrix_set(rotmat, 1, 0, 0.0)) failed = true;
    if (matrix_set(rotmat, 1, 1, c)) failed = true;
    if (matrix_set(rotmat, 1, 2, s)) failed = true;
    if (matrix_set(rotmat, 2, 0, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 1, -s)) failed = true;
    if (matrix_set(rotmat, 2, 2, c)) failed = true;

    if (failed) {
        matrix_free(rotmat);
        return ERR("Failed to construct rotation matrix");
    } else {
        return NO_ERROR;
    }
}

/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_y(matrix_t *rotmat, const double rad) {
    // Local variables
    err_t status = NO_ERROR;    //     Error code
    bool failed = false;        //     Flag to indicate failure of set operation
    const double c = cos(rad);  //     Cosine of the rotation angle
    const double s = sin(rad);  //     Sine of the rotation angle

    status = matrix_init(rotmat, 3, 3);
    if (status != NO_ERROR) return ERR("Failed to initialize rotation matrix");

    if (matrix_set(rotmat, 0, 0, c)) failed = true;
    if (matrix_set(rotmat, 0, 1, 0.0)) failed = true;
    if (matrix_set(rotmat, 0, 2, -s)) failed = true;
    if (matrix_set(rotmat, 1, 0, 0.0)) failed = true;
    if (matrix_set(rotmat, 1, 1, 1.0)) failed = true;
    if (matrix_set(rotmat, 1, 2, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 0, s)) failed = true;
    if (matrix_set(rotmat, 2, 1, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 2, c)) failed = true;

    if (failed) {
        matrix_free(rotmat);
        return ERR("Failed to construct rotation matrix");
    } else {
        return NO_ERROR;
    }
}

/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_z(matrix_t *rotmat, const double rad) {
    // Local variables
    err_t status = NO_ERROR;    //     Error code
    bool failed = false;        //     Flag to indicate failure of set operation
    const double c = cos(rad);  //     Cosine of the rotation angle
    const double s = sin(rad);  //     Sine of the rotation angle

    status = matrix_init(rotmat, 3, 3);
    if (status != NO_ERROR) return ERR("Failed to initialize rotation matrix");

    if (matrix_set(rotmat, 0, 0, c)) failed = true;
    if (matrix_set(rotmat, 0, 1, s)) failed = true;
    if (matrix_set(rotmat, 0, 2, 0.0)) failed = true;
    if (matrix_set(rotmat, 1, 0, -s)) failed = true;
    if (matrix_set(rotmat, 1, 1, c)) failed = true;
    if (matrix_set(rotmat, 1, 2, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 0, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 1, 0.0)) failed = true;
    if (matrix_set(rotmat, 2, 2, 1.0)) failed = true;

    if (failed) {
        matrix_free(rotmat);
        return ERR("Failed to construct rotation matrix");
    } else {
        return NO_ERROR;
    }
}

/* ------------------------------------------------------------------------- */
