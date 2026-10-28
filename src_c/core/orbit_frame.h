#ifndef ORBIT_FRAME_H
#define ORBIT_FRAME_H

#include "error_handling.h"


typedef enum {
    ORBIT_FRAME_J2000,
    ORBIT_FRAME_TEME,
    ORBIT_FRAME_ITRF,
    ORBIT_FRAME_ICRF,
    ORBIT_FRAME_TOD,
    ORBIT_FRAME_TIRS,
    ORBIT_FRAME_CIRS,
    ORBIT_FRAME_EFG
} orbit_frame_t;


/* ------------------------------------------------------------------------- */

#endif
