#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdbool.h>

#include "working_dir.h"
#include "parse_cmdline.h"
#include "block_graph.h"
#include "core/error_handling.h"

// Module data structures -----------------------------------------------------

// ----------------------------------------------------------------------------
err_t environment_init(int argc, char *argv[]);
err_t environment_done();

#endif // ENVIRONMENT_H
