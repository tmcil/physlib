#include "error_handling.h"

// Module Variables -----------------------------------------------------------
static FILE *stdout_file                = NULL;
static FILE *stderr_file                = NULL;

// ----------------------------------------------------------------------------
err_t debug_msg(const err_t severity, const char *filepath, const int line,
                const char *message, ...) {
    // Local variables
    char line_buffer[512];
    va_list args, copy;
    const char* filename;
    
    va_start(args, message);
    // Extract the filename from the __FILE__ macro, it is the last element
    // in the path.
    filename = strrchr(filepath, '/');
    filename = (filename) ? filename + 1 : filepath;

    va_copy(copy, args);
    construct_debug_message(line_buffer, 512, severity, filename, line, 
                            message, copy);
    va_end(copy);
    va_end(args);

    // Print to stdout.
    printf("%s", line_buffer);

    // Print to stdout file.
    if (stdout_file) {
        fputs(line_buffer, stdout_file);
        fflush(stdout_file);
    }

    // Error calls are also written to stderr file.
    if (stderr_file && severity == ERROR) {
        fputs(line_buffer, stderr_file);
        fflush(stderr_file);
    }

    return severity;
}

// ----------------------------------------------------------------------------
void msg(const char *message, ...) {
    // Local variables
    char line_buffer[512];
    va_list args, copy;

    va_start(args, message);
    va_copy(copy, args);
    construct_message(line_buffer, 512, NO_ERROR, message, copy);
    va_end(copy);
    va_end(args);
    printf("%s", line_buffer);

    if (stdout_file) {
        fputs(line_buffer, stdout_file);
        fflush(stdout_file);
    }

    return;
}

// ----------------------------------------------------------------------------
err_t open_stdout_file(const char *path) {

    if (stdout_file) return ERR("STDOUT file is already initialised");
    
    stdout_file = fopen(path, "w");
    if (!stdout_file) return ERR("Failed to initialse STDOUT file");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t open_stderr_file(const char *path) {

    if (stderr_file) return ERR("STDERR file is already initialised");
    
    stderr_file = fopen(path, "w");
    if (!stderr_file) return ERR("Failed to initialse STDERR file");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t close_stdout_file() {
    if (!stdout_file) return NO_ERROR;

    if (fclose(stdout_file) == EOF) {
        return ERR("Failed to close STDOUT file");
    }
    
    stdout_file = NULL;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t close_stderr_file() {
    if (!stderr_file) return NO_ERROR;

    if (fclose(stderr_file) == EOF) {
        return ERR("Failed to close STDERR file");
    }
    
    stderr_file = NULL;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t construct_message(char *constructed, const size_t len_constructed, 
                        const err_t error, const char *msg, va_list args){
    // Local variables
    char error_char;
    struct tm utc;
    char timestamp[21];
    char formatted_msg[512];
    int stat;
    size_t tstat;
    // Local constants
    const time_t now = time(NULL);

    // Format the time
    gmtime_r(&now, &utc);
    tstat = strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &utc);
    if (tstat == 0) return WAR("Poorly formatted timestamp: %s", timestamp);

    // Select the correct level character, based on the err_t.
    switch(error) {
        case NO_ERROR:
            error_char = 'I';
            break;
        case ERROR:
            error_char = 'E';
            break;
        case WARNING:
            error_char = 'W';
            break;
        default:
            error_char = ' ';
            return ERR("Unsupported err_t value");
        }

    // Format msg using the supplied variadic arguments.
    stat = vsnprintf(formatted_msg, sizeof(formatted_msg), msg, args);
    if (stat < 0) return ERR("Failed to format message");
    if ((size_t)stat >= sizeof formatted_msg) {
        return WAR("Message was truncated");
    }

    // Allocate the constructed string with the data filled in.
    stat = snprintf(constructed, len_constructed, "[%s] [%c] %s\n", timestamp,
                    error_char, formatted_msg);
    if (stat < 0) return ERR("Failed to construct message");
    if ((size_t)stat >= len_constructed) return WAR("Message was truncated");

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t construct_debug_message(char *constructed, const size_t len_constructed, 
                              const err_t error, const char *filename,
                              const int line, const char *msg, va_list args){
    // Local variables
    char error_char;
    struct tm utc;
    char timestamp[21];
    char formatted_msg[512];
    int stat;
    size_t tstat;
    // Local constants
    const time_t now = time(NULL);

    // Format the time
    gmtime_r(&now, &utc);
    tstat = strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &utc);
    if (tstat == 0) return WAR("Poorly formatted timestamp: %s", timestamp);

    // Select the correct level character, based on the err_t.
    switch(error) {
        case NO_ERROR:
            error_char = 'I';
            break;
        case ERROR:
            error_char = 'E';
            break;
        case WARNING:
            error_char = 'W';
            break;
        default:
            error_char = ' ';
            return ERR("Unsupported err_t value");
    }

    // Format msg using the supplied variadic arguments.
    stat = vsnprintf(formatted_msg, sizeof(formatted_msg), msg, args);
    if (stat < 0) return ERR("Failed to format message");
    if ((size_t)stat >= sizeof formatted_msg) {
        return WAR("Message was truncated");
    }

    // Allocate the constructed string with the data filled in.
    stat = snprintf(constructed, len_constructed, "[%s] [%c] [%s ln%d] %s\n",
                    timestamp, error_char, filename, line, formatted_msg);
    if (stat < 0) return ERR("Failed to construct message");
    if ((size_t)stat >= len_constructed) return WAR("Message was truncated");

    return NO_ERROR;
}
