#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

/**
 * Author: Thomas McIlwraith
 * 
 * This module exposes some convenient error handling macros and definitions.
 *   MSG(str) - Write a level 0 (NO_ERROR/INFO) message to the stdout
 *   ERR(str) - Write a level 1 (ERROR) debug message to the stdout
 *   WAR(str) - Write a level 2 (WARNING) debug message to the stdout
 *
 * All log messages are passed as variable arg lists, meaning that you can pass
 * string insertion candidates without clogging up the parent scope.
 *
 * Additionally, WAR and ERR can be used directly as the corresponding return
 * codes. This reduces the number of lines required on each check by one. If it
 * is more convenient, the err_t codes can be returned directly. See example:
 * 
 * ...
 * err_t error = pythagoras(hyp, side1, side2);
 * if (error) return ERR("Bad triangle: %f %f", side1, side2);
 *
 * return NO_ERROR;
 * ...
 * 
 * The library provides functions for opening/closing stdout/err files. Log
 * messages will also be piped to the relevant file buffers if they are open
 * automatically.
 * 
 * WAR and ERR messages return a debug statement including filename and line
 * number of the offence.
 * 
 * stdout and stderr files can be created with open_stdout_file() and
 * open_stderr_file(), respectively. Make sure to close these once you are
 * done.
*/

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

typedef enum {
    NO_ERROR,
    ERROR,
    WARNING
} err_t;

// Write a level 0 message to all open outputs.
#define MSG(...) msg(__VA_ARGS__)
// Write a level 2 debug messaage to all open outputs, included stderr.
#define ERR(...) debug_msg(ERROR, __FILE__, __LINE__, __VA_ARGS__)
// Write a level 1 debug messaage to all open outputs.
#define WAR(...) debug_msg(WARNING, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Open a stdout file. Only one can be open at runtime.
 * @param path File path to dump stdout into.
 */
err_t open_stdout_file(const char *path);

/**
 * @brief Open a stderr file. Only one can be open at runtime.
 * @param path File path to dump stderr into.
 */
err_t open_stderr_file(const char *path);

/**
 * @brief Close a stdout file.
 */
err_t close_stdout_file();

/**
 * @brief Close a stderr file.
 */
err_t close_stderr_file();

// ----------------------------------------------------------------------------
// Below should only be used internally, within this module.

err_t debug_msg(const err_t severity, const char *filepath, const int line,
                const char *message, ...);

// Write a message to the outputs.
void msg(const char *message, ...);

err_t construct_message(char *constructed, const size_t len_constructed, 
                        const err_t error, const char *msg, va_list args);
err_t construct_debug_message(char *constructed, const size_t len_constructed, 
                              const err_t error, const char *filename,
                              const int line, const char *msg, va_list args);

#endif // ERROR_HANDLING_H
