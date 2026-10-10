#ifndef PARSE_CMDLINE_H
#define PARSE_CMDLINE_H

/**
 * Author: Thomas McIlwraith
 */

#include <ctype.h>
#include <string.h>

#include "utils/error_handling.h"
#include "utils/uuid.h"

// Module data structures -----------------------------------------------------

typedef struct {
    uuid_t execution_id;
    char *block_name;
    uuid_t block_id;
} cmdline_args_t;


extern cmdline_args_t g_cmdline_args;
#define N_CMDLINE_ARGS 3

// ----------------------------------------------------------------------------

err_t cmdline_args_read(int argc, char *argv[]);

#endif // PARSE_CMDLINE_H
