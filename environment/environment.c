#include "environment.h"

// Module Variables -----------------------------------------------------------

// ----------------------------------------------------------------------------
err_t environment_init(int argc, char *argv[]){
    // Local variables
    err_t err = NO_ERROR;
    char log_path[512];
    cJSON *j_block_graph = NULL;

    // Read the cmdline arguements and set g_cmdline_args
    err = cmdline_args_read(argc, argv);
    if (err) return ERR("Failed to parse command line arguments");

    // Set the working directory paths global variables g_working_dir_xxx
    err = working_dir_set();
    if (err) return ERR("Failed to set working dir paths");

    // Read the block_graph.json and set g_block_graph
    err = working_dir_read(&j_block_graph, "inputs/block_graph.json");
    if (err) return ERR("Failed to read block graph JSON");
    err = graph_read_json(&g_block_graph, j_block_graph);
    if (err) return ERR("Malformed block graph JSON");
    cJSON_Delete(j_block_graph);

    // Open the stdout file
    err = working_dir_file(log_path, 512, LOGS, true, false);
    if (err) return ERR("Failed to create stdout path");
    open_stdout_file(log_path);

    // Open the stdout file
    err = working_dir_file(log_path, 512, LOGS, false, true);
    if (err) return ERR("Failed to create stderr path");
    open_stderr_file(log_path);

    MSG("Block started: %s", g_cmdline_args.block_name);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t environment_done() {
    // Local variables
    err_t err = NO_ERROR;

    graph_reset(&g_block_graph);

    err = close_stdout_file();
    if (err) return ERR("Failed to close stdout file");

    err = close_stderr_file();
    if (err) return ERR("Failed to close stderr file");

    MSG("Block finished: %s", g_cmdline_args.block_name);

    return NO_ERROR;
}
