
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

void cartsv_free(cartsv_t *sv) {
    sv->frame = ORBIT_FRAME_J2000;
    epoch_zero(&sv->epoch);
    vector_free(&sv->r);
    vector_free(&sv->v);
    matrix_free(&sv->cov);
    return;
}

/* ------------------------------------------------------------------------- */

err_t cartsv_array_init(cartsv_array_t *trajectory, const size_t capacity) {
    size_t i = 0;
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
    return (trajectory->statevectors != NULL && trajectory->capacity != 0 &&
            trajectory->n_statevectors != 0);
}

/* ------------------------------------------------------------------------- */

void cartsv_array_free(cartsv_array_t *trajectory) {
    size_t i = 0;
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
    cartsv_t *aux = NULL;
    size_t new_capacity = 0;
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

    trajectory->statevectors[trajectory->n_statevectors] = statevector;
    trajectory->n_statevectors += 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */
