
#include "constants.h"


/* ------------------------------------------------------------------------- */

err_t planetary_constants(planetary_constants_t *constants, center_body_t body) {
    if (constants == NULL) return ERR("NULL constants argument");
    switch (body) {
        case EARTH:
            constants->radius = 6378.1363;
            constants->rotation_rate = 7.292115e-5;
            break;
        default:
            return ERR("Invalid center body :%d", (int)body);
    }
}

/* ------------------------------------------------------------------------- */
