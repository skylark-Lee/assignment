#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pipe.h"
#include "execute.h"
#include "error.h"

struct pipe_command *parse_pipe(char *line) {
    if (!line) return NULL;
    
    struct pipe_command *pcmd = malloc(sizeof(struct pipe_command));
    if (!pcmd) return NULL;
    
    pcmd->cmd_count = 0;
    char *line_copy = strdup(line);
    char *saveptr = NULL;
    char *token = strtok_r(line_copy, "|", &saveptr);
    
    while (token && pcmd->cmd_count < MAX_PIPES) {
        char *cmd_str = token;
        while (*cmd_str == ' ' || *cmd_str == '\t') cmd_str++;
        char *end = cmd_str + strlen(cmd_str) - 1;
        while (end > cmd_str && (*end == ' ' || *end == '\t')) end--;
        *(end + 1) = '\0';
        
        if (strlen(cmd_str) > 0) {
            pcmd->commands[pcmd->cmd_count] = parse_command(cmd_str);
            if (pcmd->commands[pcmd->cmd_count]) {
                pcmd->cmd_count++;
            }
        }
        token = strtok_r(NULL, "|", &saveptr);
    }
    
    free(line_copy);
    return pcmd;
}

int execute_pipe(struct pipe_command *pcmd) {
    if (!pcmd || pcmd->cmd_count == 0) {
        xshell_error(ERR_SYNTAX, "empty pipe");
        return 1;
    }
    
    if (pcmd->cmd_count == 1) {
        return execute(pcmd->commands[0]);
    }
    
    int pipefds[2 * (pcmd->cmd_count - 1)];
    pid_t pids[pcmd->cmd_count];
    
    for (int i = 0; i < pcmd->cmd_count - 1; i++) {
        if (pipe(pipefds + 2*i) == -1) {
            xshell_error(ERR_PIPE_FAILED, "failed to create pipe");
            perror("pipe");
            return 1;
        }
    }
    
    for (int i = 0; i < pcmd->cmd_count; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i > 0) dup2(pipefds[2*(i-1)], STDIN_FILENO);
            if (i < pcmd->cmd_count - 1) dup2(pipefds[2*i + 1], STDOUT_FILENO);
            
            for (int j = 0; j < 2 * (pcmd->cmd_count - 1); j++) {
                close(pipefds[j]);
            }
            
            int result = execute(pcmd->commands[i]);
            exit(result);
        } else if (pids[i] == -1) {
            xshell_error(ERR_FORK_FAILED, "failed to create child process");
            perror("fork");
            
            for (int j = 0; j < 2 * (pcmd->cmd_count - 1); j++) {
                close(pipefds[j]);
            }
            return 1;
        }
    }
    
    for (int i = 0; i < 2 * (pcmd->cmd_count - 1); i++) {
        close(pipefds[i]);
    }
    
    int status;
    for (int i = 0; i < pcmd->cmd_count; i++) {
        waitpid(pids[i], &status, 0);
    }
    
    return WEXITSTATUS(status);
}

void free_pipe_command(struct pipe_command *pcmd) {
    if (!pcmd) return;
    for (int i = 0; i < pcmd->cmd_count; i++) {
        if (pcmd->commands[i]) free_command(pcmd->commands[i]);
    }
    free(pcmd);
}