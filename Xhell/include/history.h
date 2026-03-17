#ifndef HISTORY_H
#define HISTORY_H

void init_history(void);
void add_history(const char *command);
void show_history(void);
void free_history(void);

// 新增：历史导航功能
char *get_previous_history(void);
char *get_next_history(void);
void reset_history_position(void);
int get_history_count(void);
char *get_history_at(int index);

#endif