#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <stdarg.h>

//typedef enum {
//    ERR_SYNTAX = 1,
//    ERR_CMD_NOT_FOUND,
//    ERR_FILE_NOT_FOUND,
//    ERR_PERMISSION_DENIED,
//    ERR_INVALID_ARGUMENT,
//    ERR_MEMORY_ALLOC,
//    ERR_PIPE_FAILED,
//    ERR_FORK_FAILED,
//    ERR_REDIRECT_FAILED
//} error_type_t;
typedef enum {
    ERR_SYNTAX,
    ERR_CMD_NOT_FOUND,
    ERR_FILE_NOT_FOUND,
    ERR_PERMISSION_DENIED,
    ERR_INVALID_ARGUMENT,
    ERR_MEMORY_ALLOC,
    ERR_PIPE_FAILED,
    ERR_FORK_FAILED,
    ERR_REDIRECT_FAILED,
    ERR_GENERAL
} error_type_t;

void xshell_error(error_type_t type, const char *msg);
void log_error(const char *format, ...);

#endif