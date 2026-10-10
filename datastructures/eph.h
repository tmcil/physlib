#ifndef EPH_H
#define EPH_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>
#include <stdlib.h>

#include "utils/error_handling.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

typedef struct {
    // Epoch in MJD2000
    double epoch;
    // 3D state vector [x1, x2, x3, v1, v2, v3]
    double vec[6];
} vec_t;

typedef struct {
    // Epoch in MJD2000
    double epoch;
    // Lower triangular covariance matrix
    double cov[21];
} cov_t;

typedef struct {
    // Related satellite ID, -1 if not associated to an object
    int satellite_id;
    // Central body
    int center_body;
    // Reference Frame
    int reference_frame;
    // Scale factor of units. SF x val = val expressed in meters
    double meters_scale_factor;
    // Number of datapoints in both statevector and covariance arrays
    int n_datapoints;
    // True if state vector information is present
    bool has_state_vectors;
    // True if covariance information is present
    bool has_covariance_matrices;
    // Array of state vectors, always has dimension n_datapoints
    vec_t *state_vectors;
    // Array of covariances, always has dimension n_datapoints
    cov_t *covariance_matrices;
} eph_t;

// ----------------------------------------------------------------------------

err_t eph_init(eph_t *item, const int satellite_id, const int center_body,
               const int reference_frame, const double meters_scale_factor);
err_t eph_push(eph_t *item, const vec_t *vec, const cov_t *cov,
              const bool add_vec, const bool add_cov);
err_t eph_create_json(cJSON **json, const eph_t *item);
err_t eph_read_json(eph_t *item, const cJSON *json);

err_t eph_reset(eph_t *item);
bool eph_isreset(const eph_t *item);

#endif // EPH_H
