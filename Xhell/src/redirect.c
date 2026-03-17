#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "redirect.h"

static int saved_stdin = -1;
static int saved_stdout = -1;
static int saved_stderr = -1;

int setup_redirection(struct command *cmd) {
    // Save original file descriptors
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);
    
    if (saved_stdin == -1 || saved_stdout == -1 || saved_stderr == -1) {
        fprintf(stderr, "Error: Failed to save original file descriptors\n");
        perror("dup");
        return -1;
    }
    
    // Handle input redirection
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Error: Cannot open input file '%s'\n", cmd->input_file);
            perror("open");
            return -1;
        }
        
        if (dup2(fd, STDIN_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to redirect stdin\n");
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // Handle output redirection
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->append_mode) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        int fd = open(cmd->output_file, flags, 0644);
        if (fd == -1) {
            fprintf(stderr, "Error: Cannot open output file '%s'\n", cmd->output_file);
            perror("open");
            return -1;
        }
        
        if (dup2(fd, STDOUT_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to redirect stdout\n");
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // Handle error redirection
    if (cmd->error_file) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->error_append_mode) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        int fd = open(cmd->error_file, flags, 0644);
        if (fd == -1) {
            fprintf(stderr, "Error: Cannot open error file '%s'\n", cmd->error_file);
            perror("open");
            return -1;
        }
        
        if (dup2(fd, STDERR_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to redirect stderr\n");
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // Handle stderr redirection to stdout
    if (cmd->redirect_stderr_to_stdout) {
        if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to redirect stderr to stdout\n");
            perror("dup2");
            return -1;
        }
    }
    
    return 0;
}

int restore_redirection(void) {
    int ret = 0;
    
    // Restore stdin
    if (saved_stdin != -1) {
        if (dup2(saved_stdin, STDIN_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to restore stdin\n");
            perror("dup2");
            ret = -1;
        } else {
            close(saved_stdin);
            saved_stdin = -1;
        }
    }
    
    // Restore stdout
    if (saved_stdout != -1) {
        if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to restore stdout\n");
            perror("dup2");
            ret = -1;
        } else {
            close(saved_stdout);
            saved_stdout = -1;
        }
    }
    
    // Restore stderr
    if (saved_stderr != -1) {
        if (dup2(saved_stderr, STDERR_FILENO) == -1) {
            fprintf(stderr, "Error: Failed to restore stderr\n");
            perror("dup2");
            ret = -1;
        } else {
            close(saved_stderr);
            saved_stderr = -1;
        }
    }
    
    return ret;
}