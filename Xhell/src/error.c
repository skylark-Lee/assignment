//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <time.h>
//#include <string.h>
//#include <unistd.h>
//#include "error.h"
//
//void xshell_error(error_type_t type, const char *msg) {
//    const char *type_str;
//    
//    switch (type) {
//        case ERR_SYNTAX: type_str = "grammar mistack"; break;
//        case ERR_CMD_NOT_FOUND: type_str = "command no found"; break;
//        case ERR_FILE_NOT_FOUND: type_str = "file do not exist"; break;
//        case ERR_PERMISSION_DENIED: type_str = "insufficient privileges"; break;
//        case ERR_INVALID_ARGUMENT: type_str = "parameter error"; break;
//        case ERR_MEMORY_ALLOC: type_str = "memory allocation failure"; break;
//        case ERR_PIPE_FAILED: type_str = "pipe creation failed"; break;
//        case ERR_FORK_FAILED: type_str = "process creation failed"; break;
//        case ERR_REDIRECT_FAILED: type_str = "redirection failed"; break;
//        default: type_str = "unknown error"; break;
//    }
//    
//    // ʹ��perror�������׼������
//    if (msg) {
//        fprintf(stderr, "xshell: %s: %s\n", type_str, msg);
//    } else {
//        fprintf(stderr, "xshell: %s\n", type_str);
//    }
//    
//    // ��¼��ϸ��־
//    log_error("[mistake] type:%d message:%s", type, msg ? msg : "null");
//}
//
//void log_error(const char *format, ...) {
//    FILE *log_file = fopen("xshell.log", "a");
//    if (!log_file) return;
//    
//    time_t now = time(NULL);
//    char time_str[64];
//    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
//    
//    va_list args;
//    va_start(args, format);
//    
//    fprintf(log_file, "[%s] PID:%d ", time_str, getpid());
//    vfprintf(log_file, format, args);
//    fprintf(log_file, "\n");
//    
//    va_end(args);
//    fclose(log_file);
//}
// 在 error.c 中增强错误处理
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error.h"
#include <stdarg.h> 

void xshell_error(error_type_t type, const char *msg) {
    const char *type_str = NULL;
    
    switch (type) {
        case ERR_SYNTAX: type_str = "grammar mistack"; break;
        case ERR_CMD_NOT_FOUND: type_str = "command no found"; break;
        case ERR_FILE_NOT_FOUND: type_str = "file do not exist"; break;
        case ERR_PERMISSION_DENIED: type_str = "insufficient privileges"; break;
        case ERR_INVALID_ARGUMENT: type_str = "parameter error"; break;
        case ERR_MEMORY_ALLOC: type_str = "memory allocation failure"; break;
        case ERR_PIPE_FAILED: type_str = "pipe creation failed"; break;
        case ERR_FORK_FAILED: type_str = "process creation failed"; break;
        case ERR_REDIRECT_FAILED: type_str = "redirection failed"; break;
        case ERR_GENERAL: type_str = "mistake"; break;
        default: type_str = "unknow mistack"; break;
    }
    
    // 统一格式：xshell: 错误类型: 具体信息
    fprintf(stderr, "mistake type: %s ", type_str);
    
    if (msg) {
        fprintf(stderr, "%s", msg);
    }
    
    // 如果有系统错误，显示errno信息
    if (errno != 0 && type != ERR_CMD_NOT_FOUND) {
        fprintf(stderr, " (%s)", strerror(errno));
    }
    
    fprintf(stderr, "\n");
//    log_error("[mistake] type:%d message:%s", type, msg ? msg : "null");
}
//void log_error(const char *format, ...) {
//    FILE *log_file = fopen("xshell.log", "a");
//    if (!log_file) return;
//    
//    time_t now = time(NULL);
//    char time_str[64];
//    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
//    
//    va_list args;
//    va_start(args, format);
//    
//    fprintf(log_file, "[%s] PID:%d ", time_str, getpid());
//    vfprintf(log_file, format, args);
//    fprintf(log_file, "\n");
//    
//    va_end(args);
//    fclose(log_file);
//}