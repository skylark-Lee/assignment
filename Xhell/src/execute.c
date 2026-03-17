#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "execute.h"
#include "builtin.h"
#include "redirect.h"
#include "error.h"

int check_command_exists(const char *cmd) {
    if (strchr(cmd, '/')) {
        return access(cmd, X_OK) == 0;
    } else {
        char *path = getenv("PATH");
        if (!path) return 0;
        
        char *path_copy = strdup(path);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
            if (access(full_path, X_OK) == 0) {
                free(path_copy);
                return 1;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
        return 0;
    }
}
int execute_external(struct command *cmd) {
    if (!cmd || cmd->argc == 0) {
        xshell_error(ERR_SYNTAX, "emptyn command");
        return 1;
    }
    
    // 检查命令是否存在
    if (!check_command_exists(cmd->args[0])) {
        xshell_error(ERR_CMD_NOT_FOUND, cmd->args[0]);
        return 127;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        xshell_error(ERR_FORK_FAILED, "failed to create process");
        perror("fork");
        return 1;
    }
    
    if (pid == 0) {
        // 子进程
        execvp(cmd->args[0], cmd->args);
        
        // execvp失败
        if (errno == ENOENT) {
            xshell_error(ERR_CMD_NOT_FOUND, cmd->args[0]);
        } else if (errno == EACCES) {
            xshell_error(ERR_PERMISSION_DENIED, cmd->args[0]);
        } else {
            perror(cmd->args[0]);
        }
        exit(127);
    } else {
        // 父进程
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            xshell_error(ERR_GENERAL, "The process was terminated by a signal");
            return 128 + WTERMSIG(status);
        }
    }
    
    return 0;
}

int execute(struct command *cmd) {
    if (cmd->argc == 0) {
        return 0;
    }
    //printf("debug: execute '%s',parameters: %d\n", cmd->args[0], cmd->argc);
    if (cmd->error_file) {
        //printf("debug: redirect the error to a file: %s\n", cmd->error_file);
    }
    // 第一步：设置重定向
    if (setup_redirection(cmd) != 0) {
        fprintf(stderr, "mistake: redirect setting failed\n");
        return 1;
    }
    
    int result = 0;
    builtin_func builtin = get_builtin(cmd->args[0]);
    
    if (builtin) {
        // 内置命令
        result = builtin(cmd->args);
    } else {
        // 外部命令
        result = execute_external(cmd);
    }
    
    // 第二步：恢复重定向
    restore_redirection();
    
    return result;
}