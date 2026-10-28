#ifndef FILE_IO_H
#define FILE_IO_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>

#include "core/error_handling.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

// ----------------------------------------------------------------------------

err_t read_json(cJSON **json, const char *filepath);
err_t write_json(const cJSON *json, const char *filepath);
char *read_file(const char *filename);

#endif // FILE_IO_H
