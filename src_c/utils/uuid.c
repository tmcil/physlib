#include "uuid.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
bool uuid_eq(uuid_t u1, uuid_t u2) {
    return (strcmp(uuid_tostring(u1), uuid_tostring(u2)) == 0);
}

// ----------------------------------------------------------------------------
err_t uuid_set(uuid_t *u, const char* val) {
    // Local variables
    err_t err = NO_ERROR;

    u->val = strdup(val);
    if (u->val == NULL) return ERR("Failed to allocate memory");

    err = uuid_check(u);
    if (err) return ERR("Invalid UUID cmdline arguement");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t uuid_check(const uuid_t *u) {

    if (u->val[8] != '-' || u->val[13] != '-' || u->val[18] != '-' ||
        u->val[23] != '-') {
            return ERR("Invalid dash (-) placement in UUID");
        }
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
char* uuid_tostring(const uuid_t u) {
    if (u.val == NULL) return NULL;
    return strdup(u.val);
}

// ----------------------------------------------------------------------------
char* uuid_tostring_short(const uuid_t u) {
    // Local variables
    char *short_uuid = NULL;
    size_t len = 0;

    if (u.val == NULL) return NULL;
    len = strlen(u.val);
    if (len < 12) return NULL;

    short_uuid = malloc(13);
    if (short_uuid == NULL) return NULL;

    memcpy(short_uuid, u.val + len - 12, 12);
    short_uuid[12] = '\0';

    return short_uuid;
}

// ----------------------------------------------------------------------------
void uuid_reset(uuid_t *u) {
    if (u->val != NULL) free(u->val);
    return;
}
