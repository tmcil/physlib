
#include "cartsv.h"


/* ------------------------------------------------------------------------- */

int cartsv_init(cartsv_t *sv, const int dims, const orbit_frame_t frame) {
    if (sv == NULL) return 1;
    sv->frame = frame;
    epoch_zero(&sv->epoch);
    if (vector_init(&sv->r, dims)) return 1;
    if (vector_init(&sv->v, dims)) return 1;
    if (matrix_init_identity(&sv->cov, dims*2)) return 1;
    return 0;
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
