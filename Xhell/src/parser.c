#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

// 移除参数两边的引号
char *remove_quotes(char *str) {
    if (!str || strlen(str) < 2) return str;
    
    int len = strlen(str);
    char *result = str;
    
    // 检查双引号
    if (str[0] == '"' && str[len-1] == '"') {
        str[len-1] = '\0';
        result = str + 1;
    }
    // 检查单引号
    else if (str[0] == '\'' && str[len-1] == '\'') {
        str[len-1] = '\0';
        result = str + 1;
    }
    
    return result;
}

struct command *parse_command(char *line) {

    if (!line || strlen(line) == 0) {
        return NULL;
    }
    
    struct command *cmd = malloc(sizeof(struct command));
    if (!cmd) {
        perror("malloc");
        return NULL;
    }
    
    // 初始化命令结构
    memset(cmd, 0, sizeof(struct command));
    cmd->append_mode = 0;
    cmd->error_append_mode = 0;
    cmd->background = 0;
    cmd->redirect_stderr_to_stdout = 0;
    
    char *line_copy = strdup(line);
    if (!line_copy) {
        perror("strdup");
        free(cmd);
        return NULL;
    }
    
    char *saveptr;
    char *token = strtok_r(line_copy, " \t", &saveptr);
    int parsing_args = 1;
    
    while (token != NULL && cmd->argc < MAX_ARGS - 1) {
        if (parsing_args) {
            // 处理重定向符号
            if (strcmp(token, "<") == 0) {
                // 输入重定向
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    cmd->input_file = strdup(remove_quotes(token));
                }
                // 不要设置 parsing_args = 0，继续处理下一个token
            } else if (strcmp(token, ">") == 0) {
                // 输出重定向（覆盖）
                cmd->append_mode = 0;
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    cmd->output_file = strdup(remove_quotes(token));
                }
            } else if (strcmp(token, ">>") == 0) {
                // 输出重定向（追加）
                cmd->append_mode = 1;
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    cmd->output_file = strdup(remove_quotes(token));
                }
            } else if (strcmp(token, "2>") == 0) {
                // 错误重定向（覆盖）
                cmd->error_append_mode = 0;
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    cmd->error_file = strdup(remove_quotes(token));
                }
            } else if (strcmp(token, "2>>") == 0) {
                // 错误重定向（追加）
                cmd->error_append_mode = 1;
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    cmd->error_file = strdup(remove_quotes(token));
                }
            } else if (strcmp(token, "2>&1") == 0) {
                // 错误重定向到标准输出
                cmd->redirect_stderr_to_stdout = 1;
            } else if (strcmp(token, "&") == 0) {
                // 后台运行
                cmd->background = 1;
            } else {
                // 普通参数 - 移除引号
                char *clean_arg = remove_quotes(token);
                cmd->args[cmd->argc] = strdup(clean_arg);
                if (cmd->args[cmd->argc]) {
                    cmd->argc++;
                }
            }
        }
        
        // 获取下一个token
        token = strtok_r(NULL, " \t", &saveptr);
    }
    
    cmd->args[cmd->argc] = NULL;
    free(line_copy);
    
    if (cmd->argc == 0) {
        free_command(cmd);
        return NULL;
    }
    
    return cmd;
}

void free_command(struct command *cmd) {
    if (!cmd) return;
    
    for (int i = 0; i < cmd->argc; i++) {
        if (cmd->args[i]) {
            free(cmd->args[i]);
        }
    }
    
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->error_file) free(cmd->error_file);
    free(cmd);
}