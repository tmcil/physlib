#include "environment.h"

// Module Variables -----------------------------------------------------------

// ----------------------------------------------------------------------------
err_t environment_init(int argc, char *argv[]){
    // Local variables
    err_t err = NO_ERROR;
    char log_path[512];

    // Read the cmdline arguements and set g_cmdline_args
    err = cmdline_args_read(argc, argv);
    if (err) return ERR("Failed to parse command line arguments");

    // Set the working directory paths global variables g_working_dir_xxx
    err = working_dir_set();
    if (err) return ERR("Failed to set working dir paths");

    // Open the stdout file
    err = working_dir_file(log_path, 512, LOGS, true, false);
    if (err) return ERR("Failed to create stdout path");
    open_stdout_file(log_path);

    // Open the stdout file
    err = working_dir_file(log_path, 512, LOGS, false, true);
    if (err) return ERR("Failed to create stderr path");
    open_stderr_file(log_path);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t environment_done() {
    // Local variables
    err_t err = NO_ERROR;

    err = close_stdout_file();
    if (err) return ERR("Failed to close stdout file");

    err = close_stderr_file();
    if (err) return ERR("Failed to close stderr file");

    return NO_ERROR;
}
