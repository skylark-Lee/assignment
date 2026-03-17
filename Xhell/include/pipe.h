#ifndef PIPE_H
#define PIPE_H

#include "parser.h"

#define MAX_PIPES 10

struct pipe_command {
    struct command *commands[MAX_PIPES];
    int cmd_count;
};

struct pipe_command *parse_pipe(char *line);
int execute_pipe(struct pipe_command *pcmd);
void free_pipe_command(struct pipe_command *pcmd);

#endif