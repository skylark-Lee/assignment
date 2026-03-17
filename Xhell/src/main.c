#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "parser.h"
#include "execute.h"
#include "pipe.h"
#include "history.h"
#include "error.h"

// 修复：标记未使用参数
void sigint_handler(int sig) {
    (void)sig;  // 标记sig参数为已使用
    printf("\n");
    reset_history_position();
    print_prompt();
    fflush(stdout);
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, SIG_IGN);
    
    init_history();
    
    printf("=== welcome to LISIQI's XShell ===\n");
    printf("Type 'xhelp' for help, 'xexit' to quit.\n");
    printf("Function with:\n");
    printf("1.Basic Functions:\n");
    printf("file creation, reading, writing, copying, deletion, and moving\n");
    printf("\n");
    printf("2.Directory Navigation:\n");
    printf("cd,pwd,ls\n");
    printf("\n");
    printf("3.Input/Output:\n");
    printf("echo,cat,tee\n");
    printf("\n");
    printf("4.Advanced Functions:\n");
    printf("find search,word count statistics,command history\n");
    printf("\n");
    printf("5.Additional Features:\n");
    printf("Calculator\n");
    
    
        
    while (1) {
        print_prompt();
        char *line = read_line();
        if (!line) break;
        
        if (is_blank(line)) {
            free(line);
            continue;
        }
        int quote_count = 0;
        for (int i = 0; line[i]; i++) {
            if (line[i] == '"') quote_count++;
        }
        
        if (quote_count % 2 != 0) {
            xshell_error(ERR_SYNTAX, "unclosed quotation marks");
            free(line);
            continue;
        }
        // 首先检查是否为数学表达式
        if (calculate_expression(line)) {
            // 是数学表达式，已经处理完毕
            free(line);
            continue;
        }
        
        add_history(line);
        
        // 修复：使用status变量
        int status = 0;
        (void)status;//
        if (strchr(line, '|')) {
            struct pipe_command *pcmd = parse_pipe(line);
            if (pcmd) {
                status = execute_pipe(pcmd);
                free_pipe_command(pcmd);
            } else {
                fprintf(stderr, "mistake: unable to parse the pipe command\n");
                status = 1;
            }
        } else {
            struct command *cmd = parse_command(line);
            if (cmd) {
                status = execute(cmd);
                free_command(cmd);
            } else {
                fprintf(stderr, "mistake: unable to parse the command\n");
                status = 1;
            }
        }
        
//        // 这里可以记录状态日志（如果需要）
//        if (status != 0) {
//            log_error("commend exit status: %d", status);
//        }
        
        free(line);
    }
    
    free_history();
    printf("\nGoodbye!\n");
    return 0;
}