#ifndef CONSTANTS_H
#define CONSTANTS_H


/* ------------------------------------------------------------------------- */

#define PI      3.14159265358979323846
#define TWO_PI  2.0 * PI
#define HALF_PI 0.5 * PI

/* ------------------------------------------------------------------------- */

#define DEG2RAD    PI / 180.0
#define RAD2DEG    180.0 / PI
#define ARCSEC2RAD DEG2RAD / 3600.0
#define RAD2ARCSEC RAD2DEG * 3600.0

/* ------------------------------------------------------------------------- */

#define ZERO  0.0
#define ONE   1.0
#define KILO  1e3
#define MEGA  1e6
#define GIGA  1e9
#define MILLI 1e-3
#define MICRO 1e-6
#define NANO  1e-9

/* ------------------------------------------------------------------------- */

#define SECPERMIN 60.0
#define MINPERHOUR 60.0
#define HOURPERDAY 24.0
#define MINPERDAY 1440.0
#define SECPERDAY 86400.0

/* ------------------------------------------------------------------------- */

#endif  // CONSTANTS_H
