#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "utils.h"
#include "history.h"

// 原始终端设置
static struct termios original_termios;

// 设置终端为原始模式
void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_termios);
    raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// 恢复终端设置
void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// 支持历史记录的读取函数
char *read_line_with_history(void) {
    static char line[1024];
    int pos = 0;
    // 修复：移除未使用的变量
    // int history_index = -1;  // 移除这行
    char *history_cmd = NULL;
    
    enable_raw_mode();
    
    printf("\033[?25l"); // 隐藏光标
    
    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        
        if (c == '\n') { // 回车
            printf("\n");
            break;
        } else if (c == 127 || c == '\b') { // 退格
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
        } else if (c == 27) { // ESC序列（方向键）
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;
            
            if (seq[0] == '[') {
                if (seq[1] == 'A') { // 上箭头
                    history_cmd = get_previous_history();
                    if (history_cmd) {
                        // 清空当前行
                        for (int i = 0; i < pos; i++) printf("\b \b");
                        strcpy(line, history_cmd);
                        pos = strlen(line);
                        printf("%s", line);
                    }
                } else if (seq[1] == 'B') { // 下箭头
                    history_cmd = get_next_history();
                    // 清空当前行
                    for (int i = 0; i < pos; i++) printf("\b \b");
                    if (history_cmd) {
                        strcpy(line, history_cmd);
                        pos = strlen(line);
                        printf("%s", line);
                    } else {
                        line[0] = '\0';
                        pos = 0;
                    }
                } else if (seq[1] == 'C') { // 右箭头
                    // 可以添加光标移动支持
                } else if (seq[1] == 'D') { // 左箭头
                    // 可以添加光标移动支持
                }
            }
        } else if (c == '\t') { // Tab键
            // 可以添加自动完成功能
        } else if (isprint(c)) { // 可打印字符
            // 修复：解决有符号/无符号比较警告
            if (pos < (int)sizeof(line) - 1) {  // 添加类型转换
                line[pos++] = c;
                printf("%c", c);
            }
        }
    }
    
    line[pos] = '\0';
    
    printf("\033[?25h"); // 显示光标
    disable_raw_mode();
    
    return strdup(line);
}

char *read_line(void) {
    // 暂时禁用高级行编辑，使用简单的getline
    char *line = NULL;
    size_t bufsize = 0;
    
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            return NULL;
        } else {
            perror("read_line");
            exit(EXIT_FAILURE);
        }
    }
    
    line[strcspn(line, "\n")] = '\0';
    return line;
    
    // 注释掉下面的代码，暂时不使用行编辑
    /*
    // 如果支持行编辑，使用增强版本
    if (isatty(STDIN_FILENO)) {
        return read_line_with_history();
    } else {
        // 非交互模式使用简单版本
        char *line = NULL;
        size_t bufsize = 0;
        if (getline(&line, &bufsize, stdin) == -1) {
            return NULL;
        }
        line[strcspn(line, "\n")] = '\0';
        return line;
    }
    */
}
char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void print_prompt(void) {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;32m%s\033[0m$ ", cwd);
    } else {
        printf("xshell$ ");
    }
    fflush(stdout);
}

int is_blank(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}