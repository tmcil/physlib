
#include "cartsv.h"


/* ------------------------------------------------------------------------- */

err_t cartsv_init(cartsv_t *sv, const int dims, const orbit_frame_t frame) {
    if (sv == NULL) return ERR("State vector pointer is null");
    sv->frame = frame;
    epoch_zero(&sv->epoch);
    if (vector_init(&sv->r, dims)) return ERR("Failed to initialize position vector");
    if (vector_init(&sv->v, dims)) return ERR("Failed to initialize velocity vector");
    if (matrix_init_identity(&sv->cov, dims*2)) return ERR("Failed to initialize covariance matrix");
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool cartsv_is_allocated(const cartsv_t *sv) {
    if (sv == NULL) return false;
    return (vector_is_allocated(&sv->r) && vector_is_allocated(&sv->v) &&
            matrix_is_allocated(&sv->cov) && !epoch_iszerod(sv->epoch));
}
/* ------------------------------------------------------------------------- */

void cartsv_free(cartsv_t *sv) {
    sv->frame = ORBIT_FRAME_J2000;
    epoch_zero(&sv->epoch);
    vector_free(&sv->r);
    vector_free(&sv->v);
    matrix_free(&sv->cov);
    return;
}

/* ------------------------------------------------------------------------- */

err_t cartsv_copy(cartsv_t *dst, const cartsv_t src) {
    if (dst == NULL) return ERR("Destination cartsv pointer is NULL");
    if (!cartsv_is_allocated(&src)) return ERR("Source cartsv is not allocated");
    if (cartsv_is_allocated(dst) && dst == &src) return ERR("Source and destiantion are aliased");
    if (cartsv_is_allocated(dst)) cartsv_free(dst);

    if (cartsv_init(dst, src.r.dims, src.frame)) return ERR("Failed to initialise destination vector");
    memcpy(dst->cov.values, src.cov.values, src.cov.cols * src.cov.rows * sizeof(double));
    memcpy(dst->r.values, src.r.values, src.r.dims * sizeof(double));
    memcpy(dst->v.values, src.v.values, src.v.dims * sizeof(double));
    epoch_copy(&dst->epoch, src.epoch);
    dst->frame = src.frame;

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t cartsv_array_init(cartsv_array_t *trajectory, const size_t capacity) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (trajectory == NULL) return ERR("array pointer is null");
    if (capacity <= 0) return ERR("array capacity must be positive");
    if (cartsv_array_is_allocated(trajectory)) cartsv_array_free(trajectory);

    trajectory->capacity = capacity;
    trajectory->statevectors = malloc(capacity * sizeof(cartsv_t));
    if (trajectory->statevectors == NULL) {
        trajectory->capacity = 0;
        return ERR("Failed to allocate array memory");
    }
    for (i = 0; i < capacity; i++) {
        trajectory->statevectors[i].frame = ORBIT_FRAME_J2000;
        epoch_zero(&trajectory->statevectors[i].epoch);
        vector_init(&trajectory->statevectors[i].r, 3);
        vector_init(&trajectory->statevectors[i].v, 3);
        matrix_init_identity(&trajectory->statevectors[i].cov, 6);
    }

    trajectory->n_statevectors = 0;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool cartsv_array_is_allocated(const cartsv_array_t *trajectory) {
    if (trajectory == NULL) return false;
    return (trajectory->statevectors != NULL && trajectory->capacity != 0);
}

/* ------------------------------------------------------------------------- */

void cartsv_array_free(cartsv_array_t *trajectory) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (cartsv_array_is_allocated(trajectory)) {
        for (i = 0; i < trajectory->n_statevectors; i++) {
            vector_free(&trajectory->statevectors[i].r);
            vector_free(&trajectory->statevectors[i].v);
            matrix_free(&trajectory->statevectors[i].cov);
        }
        free(trajectory->statevectors);
        trajectory->statevectors = NULL;
        trajectory->capacity = 0;
        trajectory->n_statevectors = 0;
    }
}

/* ------------------------------------------------------------------------- */

err_t cartsv_array_push(cartsv_array_t *trajectory, const cartsv_t statevector) {
    // Local variables
    err_t err = NO_ERROR;     //     Error code
    cartsv_t *aux = NULL;     //     Pointer to reallocated trajectory array
    size_t new_capacity = 0;  //     Capacity after reallocation

    if (!cartsv_array_is_allocated(trajectory)) return ERR("array is not allocated");

    if (trajectory->n_statevectors > 0) {
        // If adding to an existing list, check the epoch is after the previous
        if (epoch_less(statevector.epoch,
                       trajectory->statevectors[trajectory->n_statevectors-1].epoch)) return ERR("State vector precedes last epoch");
    }

    if (trajectory->n_statevectors >= trajectory->capacity) {
        new_capacity = (trajectory->capacity == 0) ? 1 : 2 * trajectory->capacity;
        aux = realloc(trajectory->statevectors, new_capacity * sizeof(cartsv_t));
        if (aux == NULL) return ERR("Failed to realloc array memory");
        trajectory->statevectors = aux;
        trajectory->capacity = new_capacity;
    }

    err = cartsv_copy(&trajectory->statevectors[trajectory->n_statevectors], statevector);
    if (err) return ERR("Failed to copy statevector into array");
    trajectory->n_statevectors += 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */
