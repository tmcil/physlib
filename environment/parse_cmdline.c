#include "parse_cmdline.h"

// Module Variables -----------------------------------------------------------

cmdline_args_t g_cmdline_args = {
    .execution_id = {0},
    .block_name = NULL,
    .block_id = {0}
};

// ----------------------------------------------------------------------------
// Modifies state of g_cmdline_args global var.
err_t cmdline_args_read(int argc, char *argv[]) {
    // Local variables
    err_t err = NO_ERROR;
    char *program_name = NULL;
    char c;
    size_t n_char;

    if (argc != N_CMDLINE_ARGS) return ERR("Invalid number of arguments");

    // Extract executable name from argv[0]
    program_name = strrchr(argv[0], '/');

    if (program_name != NULL)
        // Skip the '/'
        program_name++;
    else
        program_name = argv[0];

    // Modify the program name to uppercase
    n_char = strlen(program_name);
    for (size_t i = 0; i < n_char; i++) {
        c = program_name[i];
        program_name[i] = toupper(c);
    }

    g_cmdline_args.block_name = program_name;

    err = uuid_set(&g_cmdline_args.execution_id, argv[1]);
    if (err) return ERR("Failed to set execution ID");

    err = uuid_set(&g_cmdline_args.block_id, argv[2]);
    if (err) return ERR("Failed to set block ID");

    return NO_ERROR;
}
