#ifndef CALCULATOR_H
#define CALCULATOR_H

// 判断是否为数学表达式
int is_math_expression(const char *str);

// 计算表达式
double evaluate_expression(const char *expr);

// 计算器命令处理
int calculate_expression(const char *expr);

#endif