#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include "utils.h"

// 运算符优先级
int precedence(char op) {
    switch (op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/':
        case '%': return 2;
        case '^': return 3;
        default: return 0;
    }
}

// 应用运算符
double apply_op(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            if (b == 0) {
                errno = EDOM;  // 除零错误
                return 0;
            }
            return a / b;
        case '%': return fmod(a, b);
        case '^': return pow(a, b);
        default: return 0;
    }
}

// 简单的表达式求值（支持 + - * /）
double evaluate_simple(const char *expr) {
    double numbers[100];
    char ops[100];
    int num_top = -1, op_top = -1;
    
    for (int i = 0; expr[i]; i++) {
        if (isspace(expr[i])) continue;
        
        if (isdigit(expr[i])) {
            // 读取数字
            double num = 0;
            while (expr[i] && isdigit(expr[i])) {
                num = num * 10 + (expr[i] - '0');
                i++;
            }
            i--;  // 回退一个字符
            numbers[++num_top] = num;
        } 
        else if (expr[i] == '(') {
            ops[++op_top] = expr[i];
        } 
        else if (expr[i] == ')') {
            while (op_top >= 0 && ops[op_top] != '(') {
                double b = numbers[num_top--];
                double a = numbers[num_top--];
                char op = ops[op_top--];
                numbers[++num_top] = apply_op(a, b, op);
            }
            op_top--;  // 弹出 '('
        } 
        else if (strchr("+-*/%^", expr[i])) {
            while (op_top >= 0 && precedence(ops[op_top]) >= precedence(expr[i])) {
                double b = numbers[num_top--];
                double a = numbers[num_top--];
                char op = ops[op_top--];
                numbers[++num_top] = apply_op(a, b, op);
            }
            ops[++op_top] = expr[i];
        }
    }
    
    // 处理剩余运算符
    while (op_top >= 0) {
        double b = numbers[num_top--];
        double a = numbers[num_top--];
        char op = ops[op_top--];
        numbers[++num_top] = apply_op(a, b, op);
    }
    
    return numbers[num_top];
}

// 判断是否为数学表达式
int is_math_expression(const char *str) {
    if (!str || strlen(str) == 0) return 0;
    
    int has_operator = 0;
    int has_digit = 0;
    
    for (int i = 0; str[i]; i++) {
        if (strchr("+-*/%^()", str[i])) has_operator = 1;
        if (isdigit(str[i])) has_digit = 1;
    }
    
    return has_operator && has_digit;
}

// 计算表达式
int calculate_expression(const char *expr) {
    if (!is_math_expression(expr)) {
        return 0;  // 不是数学表达式
    }
    
    double result = evaluate_simple(expr);
    
    if (errno == EDOM) {
        printf("wrong: it is not allowed\n");
        errno = 0;
        return 1;
    } else {
        printf("= %.6g\n", result);
        return 1;
    }
}
