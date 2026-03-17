#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64
#define MAX_COMMANDS 10

struct command {
    char *args[MAX_ARGS];
    int argc;
    char *input_file;
    char *output_file;
    char *error_file;
    int append_mode;
    int error_append_mode;
    int background;
    int redirect_stderr_to_stdout;  // 确保有这个字段
};

struct command *parse_command(char *line);
void free_command(struct command *cmd);

#endif