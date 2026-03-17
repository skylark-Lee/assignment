#ifndef UTILS_H
#define UTILS_H

#define MAX_LINE 1024
#define MAX_PATH 4096

char *read_line(void);
char *read_line_with_history(void);  // 新增：支持历史记录的读取
char *trim(char *str);
void print_prompt(void);
int is_blank(const char *str);

// 新增：计算器相关
int is_math_expression(const char *str);
double evaluate_expression(const char *expr);
int calculate_expression(const char *expr); 
#endif