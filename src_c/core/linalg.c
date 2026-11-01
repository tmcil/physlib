
#include "linalg.h"


/* ------------------------------------------------------------------------- */

err_t matrix_rotmat3_x(matrix_t *rotmat, const double rad) {
    // Local variables
    err_t status = NO_ERROR;    //     Error code
    bool failed = false;        //     Flag to indicate failure of set operation
    const double c = cos(rad);  //     Cosine of the rotation angle
    const double s = sin(rad);  //     Sine of the rotation angle

    status = matrix_allocate(rotmat, 3, 3);
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

    status = matrix_allocate(rotmat, 3, 3);
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

    status = matrix_allocate(rotmat, 3, 3);
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

err_t vector_rotate(vector_t *rotated, const matrix_t rotmat, const vector_t vec) {
    // Local variables
    err_t status = NO_ERROR; //     Error code
    int i = 0;               //     Iterator
    int j = 0;               //     Iterator
    vector_t result = {0};   // MEM rotated vector, ownership transfered to arg

    if (rotated == NULL) return ERR("Rotated vector pointer is null");
    if (!matrix_is_allocated(&rotmat)) return ERR("Rotation matrix is not allocated");
    if (!vector_is_allocated(&vec)) return ERR("Input vector is not allocated");
    if (rotmat.rows != rotmat.cols || rotmat.cols != vec.dims) {
        return ERR("Rotation matrix dimensions do not match input vector");
    }

    status = vector_allocate(&result, (int)rotmat.rows);
    if (status != NO_ERROR) return ERR("Failed to allocate rotated vector");

    for (i = 0; i < (int)rotmat.rows; i++) {
        for (j = 0; j < (int)rotmat.cols; j++) {
            result.values[i] += matrix_get(rotmat, i, j) * vec.values[j];
        }
    }

    vector_free(rotated);
    *rotated = result;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */
