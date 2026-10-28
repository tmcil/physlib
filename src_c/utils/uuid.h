#ifndef WORKING_DIR
#define WORKING_DIR

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>

#include "core/error_handling.h"

// Module data structures -----------------------------------------------------

typedef struct {
    char *val;
} uuid_t;

// ----------------------------------------------------------------------------

bool uuid_eq(uuid_t u1, uuid_t u2);
err_t uuid_set(uuid_t *u, const char* val);
err_t uuid_check(const uuid_t *u);
char* uuid_tostring(const uuid_t u);
char* uuid_tostring_short(const uuid_t u);
void uuid_reset(uuid_t *u);

#endif // WORKING_DIR
