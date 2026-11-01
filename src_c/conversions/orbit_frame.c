

#include "orbit_frame.h"


/* ------------------------------------------------------------------------- */

err_t tirs_2_itrs(cartsv_t *itrs, const cartsv_t *tirs, const orbit_conversion_params_t params) {
    // Local variables
    err_t status = NO_ERROR;         //     Error code
    cartsv_t temporary = {0};        // MEM Temporary result, protects aliasing
    double sofa_output[3][3] = {0};  //     Polar motion matrix
    matrix_t rotmat = {0};           // MEM Polar motion matrix
    const double *sofa_rows[3] = {sofa_output[0], sofa_output[1], sofa_output[2]};

    if (tirs == NULL) return ERR("NULL tirs argument");
    if (itrs == NULL) return ERR("NULL itrs argument");
    if (tirs->frame != ORBIT_FRAME_TIRS) return ERR("TIRS vector in wrong frame: %d", (int)tirs->frame);

    // Allocate the output vector
    status = cartsv_allocate(&temporary, 3, ORBIT_FRAME_ITRS);
    if (status != NO_ERROR) return ERR("Failed to allocate output vector memory");

    // Call the SOFA library to generate the rotation matrix
    iauPom00(params.x_pole, params.y_pole, params.tio_locator, sofa_output);
    matrix_init(&rotmat, sofa_rows, 3, 3);

    // Apply rotation to output position
    status = vector_rotate(&temporary.r, rotmat, tirs->r);
    if (status != NO_ERROR) return ERR("Failed to compute ITRS position");

    // Output velocity is unaffected by this transform, just copy
    status = vector_copy(&temporary.v, tirs->v);
    if (status != NO_ERROR) return ERR("Failed to compute ITRS velocity");

    // Copy the epoch and covariance to the new vector
    if (epoch_copy(&temporary.epoch, tirs->epoch)) return ERR("Failed epoch copy");
    if (matrix_copy(&temporary.cov, tirs->cov)) return ERR("Failed cov copy");

    // Finally, copy the temporary onto the output
    status = cartsv_copy(itrs, temporary);
    if (status != NO_ERROR) return ERR("Failed to copy to ITRS output");

    matrix_free(&rotmat);
    cartsv_free(&temporary);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t cirs_2_tirs(cartsv_t *tirs, const cartsv_t *cirs, const orbit_conversion_params_t params) {
    // Local variables
    err_t status = NO_ERROR;   //     Error code
    cartsv_t temporary = {0};  // MEM Temporary output result, protects aliasing
    matrix_t rotmat = {0};     // MEM Earth rotation angle about Z
    vector_t w_earth = {0};    // MEM Angular rotation rate of center body
    vector_t v_earth = {0};    // MEM w_earth x r_tirs
    vector_t aux = {0};        // MEM v_cirs - v_earth

    if (tirs == NULL) return ERR("NULL tirs argument");
    if (cirs == NULL) return ERR("NULL cirs argument");
    if (cirs->frame != ORBIT_FRAME_CIRS) return ERR("CIRS vector in wrong frame: %d", (int)cirs->frame);

    // Initialise earth rotation vector
    status = vector_allocate(&w_earth, 3);
    status = vector_set(&w_earth, 2, params.center_body.rotation_rate);
    if (status != NO_ERROR) return ERR("Failed to set earth rotation vector");

    // Allocate the output vector
    status = cartsv_allocate(&temporary, 3, ORBIT_FRAME_TIRS);
    if (status != NO_ERROR) return ERR("Failed to allocate output vector memory");

    // Perform rotation around Z by the earth rotation angle
    status = matrix_rotmat3_z(&rotmat, params.earth_rotation_angle);
    if (status != NO_ERROR) return ERR("Failed to compute ERA rotation");

    // Perform the rotation on position
    status = vector_rotate(&temporary.r, rotmat, cirs->r);
    if (status != NO_ERROR) return ERR("Failed to compute TIRS position");

    // Compute w_earth x r_tirs
    status = vector3_cross(&v_earth, w_earth, temporary.r);
    if (status != NO_ERROR) return ERR("Failed to compute w_earth x r_tirs");

    // Compute auxiliary v_cirs - v_earth
    status = vector_sub(&aux, cirs->v, v_earth);
    if (status != NO_ERROR) return ERR("Failed to compute v_cirs - v_earth");

    // Compute v_tirs
    status = vector_rotate(&temporary.v, rotmat, aux);
    if (status != NO_ERROR) return ERR("Failed to compute TIRS velocity");

    // Copy the epoch and covariance to the new vector
    if (epoch_copy(&temporary.epoch, cirs->epoch)) return ERR("Failed epoch copy");
    if (matrix_copy(&temporary.cov, cirs->cov)) return ERR("Failed cov copy");

    // Finally, copy the temporary onto the output
    status = cartsv_copy(tirs, temporary);
    if (status != NO_ERROR) return ERR("Failed to copy to TIRS output");

    vector_free(&aux);
    vector_free(&v_earth);
    vector_free(&w_earth);
    matrix_free(&rotmat);
    cartsv_free(&temporary);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t gcrs_2_cirs(cartsv_t *cirs, const cartsv_t *gcrs, const orbit_conversion_params_t params) {
    // Local variables
    err_t status = NO_ERROR;         //     Error code
    cartsv_t temporary = {0};        // MEM Temporary output. Protects aliasing
    matrix_t rotmat = {0};           // MEM Sofa outputs in rotation matrix
    double sofa_output[3][3] = {0};  //     Sofa outputs
    const double *sofa_rows[3] = {sofa_output[0], sofa_output[1], sofa_output[2]};

    if (cirs == NULL) return ERR("NULL cirs argument");
    if (gcrs == NULL) return ERR("NULL gcrs argument");
    if (gcrs->frame != ORBIT_FRAME_GCRS) return ERR("CIRS vector in wrong frame: %d", (int)cirs->frame);

    // Allocate the output vector
    status = cartsv_allocate(&temporary, 3, ORBIT_FRAME_CIRS);
    if (status != NO_ERROR) return ERR("Failed to allocate output vector memory");

    switch (params.sofa_c2i_method) {
        case 0:
            iauC2i00a(params.tt.date1, params.tt.date2, sofa_output);
            break;
        case 1:
            iauC2i00a(params.tt.date1, params.tt.date2, sofa_output);
            break;
        case 2:
            iauC2i06a(params.tt.date1, params.tt.date2, params.dX_cip, params.dY_cip, sofa_output);
            break;
        default:
            return ERR("Invalid SOFA C2I method");
    }
    matrix_init(&rotmat, sofa_rows, 3, 3);

    // Apply rotation to the output position
    status = vector_rotate(&temporary.r, rotmat, gcrs->r);
    if (status != NO_ERROR) return ERR("Failed to compute CIRS position");

    // Apply rotation to the output velocity
    status = vector_rotate(&temporary.v, rotmat, gcrs->v);
    if (status != NO_ERROR) return ERR("Failed to compute CIRS velocity");

    // TODO: Account for moving frame rotation here. currently only the largest
    // term is acounted for. Involves computing CIRS just before/after epoch and
    // estimating the rate change of the frame.

    // Copy the epoch and covariance to the new vector
    if (epoch_copy(&temporary.epoch, gcrs->epoch)) return ERR("Failed epoch copy");
    if (matrix_copy(&temporary.cov, gcrs->cov)) return ERR("Failed cov copy");

    // Finally, copy the temporary onto the output
    status = cartsv_copy(cirs, temporary);
    if (status != NO_ERROR) return ERR("Failed to copy to CIRS output");

    cartsv_free(&temporary);
    matrix_free(&rotmat);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */