#include "working_dir.h"

// Module Variables -----------------------------------------------------------

static char* m_working_dir = NULL;
static char* m_working_dir_execution = NULL;
char* g_working_dir_files = NULL;
char* g_working_dir_logs = NULL;
char* g_working_dir_inputs = NULL;
char* g_working_dir_outputs = NULL;

static int m_n_files_written = 0;

// ----------------------------------------------------------------------------
err_t working_dir_set() {
    // Local variables
    err_t err = NO_ERROR;   //     Error code
    char path_buffer[512];  //     Buffer to hold paths

    // Initialise the root
    m_working_dir = getenv(ENV_WORKING_DIR);
    if (m_working_dir == NULL) {
        return ERR("Environment variable could not be read: %s", ENV_WORKING_DIR);
    }

    // execution root path
    err = working_dir_join(path_buffer, 512, m_working_dir,
                           uuid_tostring(g_cmdline_args.execution_id), true);
    if (err) return ERR("Failed to set execution root path");
    m_working_dir_execution = strdup(path_buffer);
    if (m_working_dir_execution == NULL) return ERR("Failed to allocate path");

    // files path
    err = working_dir_join(path_buffer, 512, m_working_dir_execution,
                           "files", true);
    if (err) return ERR("Failed to set files path");
    g_working_dir_files = strdup(path_buffer);
    if (g_working_dir_files == NULL) return ERR("Failed to allocate files path");

    // inputs path
    err = working_dir_join(path_buffer, 512, m_working_dir_execution,
                           "inputs", true);
    if (err) return ERR("Failed to set inputs path");
    g_working_dir_inputs = strdup(path_buffer);
    if (g_working_dir_inputs == NULL) return ERR("Failed to allocate inputs path");

    // outputs path
    err = working_dir_join(path_buffer, 512, m_working_dir_execution,
                           "outputs", true);
    if (err) return ERR("Failed to set outputs path");
    g_working_dir_outputs = strdup(path_buffer);
    if (g_working_dir_outputs == NULL) return ERR("Failed to allocate outputs path");

    // logs path
    err = working_dir_join(path_buffer, 512, m_working_dir_execution,
                           "logs", true);
    if (err) return ERR("Failed to set logs path");
    g_working_dir_logs = strdup(path_buffer);
    if (g_working_dir_logs == NULL) return ERR("Failed to allocate logs path");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t working_dir_join(char *path_to_set, size_t len_path_to_set,
                      const char* dir, const char* subdir,
                      const bool must_exist) {
    // Local variables
    int i;           //     Number of characters in filepath
    struct stat st;  //     File stat object

    // Create the path
    i = snprintf(path_to_set, len_path_to_set, "%s/%s", dir, subdir);
    if (i < 0) return ERR("Failed to construct filepath: %s/%s", dir, subdir);
    if ((size_t)i >= len_path_to_set) return ERR("Filepath too long: %s/%s", dir, subdir);

    // Check the path exists
    if (!must_exist) return NO_ERROR;

    if (stat(path_to_set, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return ERR("directory does not exist: %s", path_to_set);
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t working_dir_read_input(cJSON **json) {
    // Local variables
    err_t err = NO_ERROR;         //     Error code
    int stat = 0;                 //     Status of snprintf call
    char file_path[512];          //     File path buffer
    char *short_uuid_buf = NULL;  // MEM Short UUID buffer

    short_uuid_buf = uuid_tostring_short(g_cmdline_args.block_id);
    if (short_uuid_buf == NULL) return ERR("Failed to retrieve UUID");

    stat = snprintf(file_path, 512, "%s/%s_%s.json", g_working_dir_inputs,
                    g_cmdline_args.block_name,
                    short_uuid_buf);
    if (stat < 0) return ERR("Failed to construct filepath");
    if ((size_t)stat >= 512) return ERR("Filepath too long");

    err = read_json(json, file_path);
    if (err) return ERR("Failed to read JSON contents");

    if (short_uuid_buf != NULL) free(short_uuid_buf);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t working_dir_read(cJSON **json,
                       const working_dir_subdirs_t subdir, const char *path){
    // Local variables
    err_t err = NO_ERROR;  //     Error code
    int stat = 0;          //     Status of snprintf call
    char full_path[512];   //     File path buffer
    char *dir = NULL;      // MEM Directory name

    if (path == NULL) return ERR("path has not been set");

    switch (subdir) {
        case INPUTS:
            dir = strdup("inputs");
            break;
        case OUTPUTS:
            dir = strdup("outputs");
            break;
        case FILES:
            dir = strdup("files");
            break;
        case LOGS:
            dir = strdup("logs");
            break;
        default:
            return ERR("Unknown subdir passed to function");
    }

    stat = snprintf(full_path, 512, "%s/%s/%s", m_working_dir_execution, dir, path);
    if (stat < 0) return ERR("Failed to construct filepath");
    if ((size_t)stat >= 512) return ERR("Filepath too long");

    err = read_json(json, full_path);
    if (err) return ERR("Failed to read JSON contents");

    if (dir != NULL) free(dir);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t working_dir_write(char **file_written,
                        const working_dir_subdirs_t subdir, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;   //     Error code
    char path_buffer[512];  //     File path buffer
    char *filename;         //     File name

    if (json == NULL) return ERR("Cannot write NULL JSON argument");
    if (g_working_dir_files == NULL) return ERR("Files directory not set");
    if (subdir == LOGS) return ERR("Use error_handling for log files");

    err = working_dir_file(path_buffer, 512, subdir, false, false);
    if (err) return ERR("Failed to create file path");

    err = write_json(json, path_buffer);
    if (err) return ERR("Failed to write file: %s", path_buffer);

    if (*file_written != NULL) free(*file_written);

    filename = strrchr(path_buffer, '/');
    filename = filename ? filename + 1 : path_buffer;
    *file_written = strdup(filename);
    if (*file_written == NULL) return ERR("Failed to allocate memory for output");

    m_n_files_written++;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t working_dir_file(char *file_path, size_t len_file_path,
                       const working_dir_subdirs_t subdir, const bool is_stdout,
                       const bool is_stderr) {
    // Local variables
    int stat = 0;                 //     Status of snprintf call
    char *directory = NULL;       //     Directory name
    char *short_uuid_buf = NULL;  // MEM Short UUID buffer

    switch (subdir) {
        case INPUTS:
            directory = g_working_dir_inputs;
            break;
        case OUTPUTS:
            directory = g_working_dir_outputs;
            break;
        case FILES:
            directory = g_working_dir_files;
            break;
        case LOGS:
            directory = g_working_dir_logs;
            break;
        default:
            return ERR("Invalid working_dir_subdirs_t requested");
    }

    if (directory == NULL) return ERR("Working directory not set");
    if (g_cmdline_args.block_name == NULL) return ERR("cmdline args not set");

    short_uuid_buf = uuid_tostring_short(g_cmdline_args.block_id);
    if (short_uuid_buf == NULL) return ERR("Failed to retrieve UUID");

    if (subdir == FILES) {
        stat = snprintf(file_path, len_file_path, "%s/%s_%02d_%s.json", directory,
                        g_cmdline_args.block_name, m_n_files_written,
                        short_uuid_buf);

    } else if (subdir == LOGS) {
        if (is_stdout) {
            stat = snprintf(file_path, len_file_path, "%s/%s_%s.stdout", directory,
                            g_cmdline_args.block_name,
                            short_uuid_buf);
        } else if (is_stderr) {
            stat = snprintf(file_path, len_file_path, "%s/%s_%s.stderr", directory,
                            g_cmdline_args.block_name,
                            short_uuid_buf);
        } else {
            return ERR("Must select either stdout or stderr in LOGS dir");
        }

    } else {
        stat = snprintf(file_path, len_file_path, "%s/%s_%s.json", directory,
                        g_cmdline_args.block_name,
                        short_uuid_buf);
    }

    if (stat < 0) return ERR("Failed to construct filepath");
    if ((size_t)stat >= len_file_path) return ERR("Filepath too long");

    free(short_uuid_buf);

    return NO_ERROR;
}

