#ifndef WORKING_DIR_H
#define WORKING_DIR_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <sys/stat.h>

#include "parse_cmdline.h"
#include "utils/error_handling.h"
#include "utils/file_io.h"
#include "external/cJSON.h"

// Module data structures -----------------------------------------------------

extern char* g_working_dir_files;
extern char* g_working_dir_logs;
extern char* g_working_dir_inputs;
extern char* g_working_dir_outputs;

#define ENV_WORKING_DIR "WORKING_DIR"

typedef enum {
    INPUTS,
    OUTPUTS,
    FILES,
    LOGS
} working_dir_subdirs_t;

// ----------------------------------------------------------------------------

err_t working_dir_set();
err_t working_dir_join(char *path_to_set, size_t len_path_to_set,
                      const char* dir, const char* subdir,
                      const bool must_exist);

err_t working_dir_read_input(cJSON **json);
err_t working_dir_read(cJSON **json,
                       const working_dir_subdirs_t subdir, const char *path);
err_t working_dir_write(char **file_written,
                        const working_dir_subdirs_t subdir, const cJSON *json);
err_t working_dir_file(char *file_path, size_t len_file_path,
                       const working_dir_subdirs_t subdir, const bool is_stdout,
                       const bool is_stderr);

#endif // WORKING_DIR_H
