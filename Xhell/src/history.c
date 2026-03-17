#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "history.h"

#define MAX_HISTORY 1000

static char *history[MAX_HISTORY];
static int history_count = 0;
static int history_position = -1;  // 新增：当前历史位置
static char *current_input = NULL; // 新增：当前输入缓存

void init_history(void) {
    history_count = 0;
    history_position = -1;
    current_input = NULL;
    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i] = NULL;
    }
}

void add_history(const char *command) {
    if (command == NULL || *command == '\0') {
        return;
    }

    // 不重复添加相同的连续命令
    if (history_count > 0 && strcmp(history[history_count-1], command) == 0) {
        return;
    }

    if (history_count >= MAX_HISTORY) {
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i - 1] = history[i];
        }
        history_count--;
    }
    
    history[history_count++] = strdup(command);
    history_position = -1; // 重置位置
}

void show_history(void) {
    printf("Command History (%d commands):\n", history_count);
    printf("==============================\n");
    for (int i = 0; i < history_count; i++) {
        printf("%4d: %s\n", i + 1, history[i]);
    }
}

// 新增：获取上一条历史命令
char *get_previous_history(void) {
    if (history_count == 0) return NULL;
    
    if (history_position == -1) {
        // 第一次按上键，保存当前输入
        if (current_input) free(current_input);
        current_input = NULL;
    }
    
    if (history_position < history_count - 1) {
        history_position++;
    }
    
    return history[history_count - 1 - history_position];
}

// 新增：获取下一条历史命令
char *get_next_history(void) {
    if (history_count == 0 || history_position == -1) return NULL;
    
    if (history_position > 0) {
        history_position--;
        return history[history_count - 1 - history_position];
    } else {
        history_position = -1;
        return current_input; // 返回原始输入
    }
}

// 新增：重置历史位置
void reset_history_position(void) {
    history_position = -1;
}

// 新增：获取历史数量
int get_history_count(void) {
    return history_count;
}

// 新增：获取指定位置的历史命令
char *get_history_at(int index) {
    if (index < 0 || index >= history_count) return NULL;
    return history[index];
}

void free_history(void) {
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    history_count = 0;
    history_position = -1;
    if (current_input) {
        free(current_input);
        current_input = NULL;
    }
}