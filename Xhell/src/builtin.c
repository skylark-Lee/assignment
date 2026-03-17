#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <ctype.h>
#include <features.h>  // 用于 _GNU_SOURCE
#include "builtin.h"
#include "history.h"
#include <errno.h>        
#include "error.h" 


struct builtin_command builtins[] = {
    {"xcd", xcd_cmd, "Change directory"},
    {"xls", xls_cmd, "List directory contents"},
    {"xpwd", xpwd_cmd, "Print working directory"},
    {"xecho", xecho_cmd, "Display message"},
    {"xcat", xcat_cmd, "Concatenate files"},
    {"xtouch", xtouch_cmd, "Create empty file"},
    {"xcp", xcp_cmd, "Copy files"},
    {"xrm", xrm_cmd, "Remove files"},
    {"xmv", xmv_cmd, "Move/rename files"},
    {"xhistory", xhistory_cmd, "Show command history"},
    {"xfind", xfind_cmd, "Find files"},
    {"xwc", xwc_cmd, "Word count"},
    {"xtee", xtee_cmd, "Tee command"},
    {"xhelp", xhelp_cmd, "Show this help"},
    {"xexit", xexit_cmd, "Exit the shell"},
    {"quit", xexit_cmd, "Exit the shell (alias)"}, 
    {NULL, NULL, NULL}
};

builtin_func get_builtin(char *name) {
    //printf("debug: check if command '%s' a built-in command \n", name);
    
    for (int i = 0; builtins[i].name != NULL; i++) {
        //printf("debug: compare '%s' with built_in command '%s'\n", name, builtins[i].name);
        if (strcmp(name, builtins[i].name) == 0) {
            //printf("debug: find built-in command '%s'\n", name);
            return builtins[i].func;
        }
    }
    
    //printf("debug: '%s' not a built-in command\n", name);
    return NULL;
}

// 修复：添加(void)参数以消除警告
int xpwd_cmd(char **args) {
    (void)args;  // 标记参数未使用
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("xpwd");
        return 1;
    }
}

int xcd_cmd(char **args) {
    char *dir = args[1];
    char *home = getenv("HOME");
    
    if (dir == NULL || strcmp(dir, "~") == 0) {
        dir = home;
    } else if (strcmp(dir, "-") == 0) {
        dir = getenv("OLDPWD");
        if (!dir) {
            fprintf(stderr, "xcd: OLDPWD not set\n");
            return 1;
        }
    }
    
    char oldpwd[1024];
    if (getcwd(oldpwd, sizeof(oldpwd)) == NULL) {
        perror("xcd");
        return 1;
    }
    
    if (chdir(dir) != 0) {
        perror("xcd");
        return 1;
    }
    
    // setenv 已经在 _GNU_SOURCE 中定义了
    setenv("OLDPWD", oldpwd, 1);
    return 0;
}
int xls_cmd(char **args) {
    char *dir = ".";
    if (args[1] != NULL) {
        dir = args[1];
    }
    
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        // 针对不同错误类型给出不同信息
        if (errno == EACCES) {
            fprintf(stderr, "xls: lnsufficient permissions unable to access '%s'\n", dir);
        } else if (errno == ENOENT) {
            fprintf(stderr, "xls: name '%s' no exist\n", dir);
        } else {
            fprintf(stderr, "xls: unable to access '%s': ", dir);
            perror("");
        }
        return 1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dp);
    return 0;
}


int xtouch_cmd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: xtouch <file>\n");
        return 1;
    }
    
    FILE *fp = fopen(args[1], "a");
    if (fp == NULL) {
        perror("xtouch");
        return 1;
    }
    
    fclose(fp);
    return 0;
}

int xecho_cmd(char **args) {
    int start = 1;
    int no_newline = 0;
    
    // 检查 -n 选项
    if (args[1] != NULL && strcmp(args[1], "-n") == 0) {
        start = 2;
        no_newline = 1;
    }
    
    int first_arg = 1;
    for (int i = start; args[i] != NULL; i++) {
        if (!first_arg) {
            printf(" ");
        }
        first_arg = 0;
        
        // 直接输出参数，不处理引号
        printf("%s", args[i]);
    }
    
    if (!no_newline) {
        printf("\n");
    }
    
    return 0;
}
int xcat_cmd(char **args) {
    if (args[1] == NULL) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            printf("%s", buffer);
        }
        return 0;
    }
    
    for (int i = 1; args[i] != NULL; i++) {
        FILE *fp = fopen(args[i], "r");
        if (fp == NULL) {
            // 简化错误信息，避免重复
            fprintf(stderr, "xcat: failed open '%s': ", args[i]);
            perror("");
            continue;
        }
        
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        
        fclose(fp);
    }
    
    return 0;
}


int xcp_cmd(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "Usage: xcp <source> <destination>\n");
        return 1;
    }
    
    FILE *src = fopen(args[1], "rb");
    if (src == NULL) {
        perror("xcp");
        return 1;
    }
    
    FILE *dst = fopen(args[2], "wb");
    if (dst == NULL) {
        perror("xcp");
        fclose(src);
        return 1;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    return 0;
}

int xrm_cmd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: xrm <file>\n");
        return 1;
    }
    
    if (remove(args[1]) != 0) {
        perror("xrm");
        return 1;
    }
    
    return 0;
}

int xmv_cmd(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "Usage: xmv <source> <destination>\n");
        return 1;
    }
    
    if (rename(args[1], args[2]) != 0) {
        perror("xmv");
        return 1;
    }
    
    return 0;
}

// 修复：添加(void)参数
int xhistory_cmd(char **args) {
    (void)args;  // 标记参数未使用
    show_history();
    return 0;
}
int xfind_cmd(char **args) {
    char *path = ".";      // Default: search current directory
    char *pattern = "*";   // Default: match all files
    
    // Parse arguments
    if (args[1] != NULL && args[2] != NULL) {
        // Two arguments: xfind <path> <pattern>
        path = args[1];
        pattern = args[2];
    } else if (args[1] != NULL) {
        // One argument: determine if it's a path or pattern
        if (strchr(args[1], '*') != NULL || strchr(args[1], '?') != NULL) {
            // Contains wildcards, assume it's a pattern
            pattern = args[1];
        } else {
            // No wildcards, assume it's a path
            path = args[1];
        }
    }
    
    printf("xfind: Searching for pattern '%s' in directory '%s'\n", path, pattern);
    
    // Open directory
    DIR *dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Error: Cannot open directory '%s'\n", path);
        perror("xfind");
        return 1;
    }
    
    struct dirent *entry;
    int found = 0;
    int count = 0;
    
    printf("Files found:\n");
    printf("==========\n");
    
    // Traverse directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files (starting with .)
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        // Use fnmatch for pattern matching
        if (fnmatch(pattern, entry->d_name, 0) == 0) {
            // Build full path
            char fullpath[1024];
            if (strcmp(path, ".") == 0) {
                snprintf(fullpath, sizeof(fullpath), "%s", entry->d_name);
            } else {
                snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            }
            
            // Get file type
            struct stat st;
            char type = '-';
            if (stat(fullpath, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    type = 'd';  // Directory
                } else if (S_ISLNK(st.st_mode)) {
                    type = 'l';  // Symbolic link
                } else if (S_ISREG(st.st_mode)) {
                    type = '-';  // Regular file
                }
            }
            
            printf("%d. [%c] %s\n", ++count, type, fullpath);
            found = 1;
        }
    }
    
    closedir(dir);
    
    if (!found) {
        printf("No files matching '%s' found\n", pattern);
    } else {
        printf("==========\n");
        printf("Total: %d files\n", count);
    }
    
    return 0;
}
int xwc_cmd(char **args) {
    int count_lines = 0, count_words = 0, count_chars = 0;
    char *filename = NULL;
    int from_stdin = 0;
    (void)from_stdin;
    
    // 解析选项
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            for (int j = 1; args[i][j] != '\0'; j++) {
                switch (args[i][j]) {
                    case 'l': count_lines = 1; break;
                    case 'w': count_words = 1; break;
                    case 'c': count_chars = 1; break;
                }
            }
        } else if (!filename) {
            filename = args[i];
        }
    }
    
    // 如果没有指定选项，默认全部统计
    if (!count_lines && !count_words && !count_chars) {
        count_lines = count_words = count_chars = 1;
    }
    
    FILE *fp = stdin;
    if (filename != NULL) {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            perror("xwc");
            return 1;
        }
    } else {
        from_stdin = 1;  // 标记来自标准输入
    }
    
    int lines = 0, words = 0, chars = 0;
    int in_word = 0;
    int ch;
    
    // 统计
    while ((ch = fgetc(fp)) != EOF) {
        chars++;
        
        if (ch == '\n') {
            lines++;
        }
        
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            if (in_word) {
                words++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }
    
    // 处理文件末尾的单词
    if (in_word) {
        words++;
    }
    
    // 输出结果
    if (count_lines) printf("lines: %d\n", lines);
    if (count_words) printf("words: %d\n", words);
    if (count_chars) printf("chars: %d\n", chars);
    
    // 如果是文件且有指定选项，显示文件名
    if (filename && (count_lines || count_words || count_chars)) {
        printf("%s", filename);
    }
    
    printf("\n");
    
    if (filename) {
        fclose(fp);
    }
    
    return 0;
}
int xtee_cmd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: xtee <file>\n");
        return 1;
    }
    
    FILE *fp = fopen(args[1], "w");
    if (fp == NULL) {
        perror("xtee");
        return 1;
    }
    
    printf("xtee: exit by print'.'single line or Ctrl+D\n");
    
    char buffer[1024];
    int line_count = 0;
    
    while (1) {
        printf("[%d] ", line_count + 1);
        fflush(stdout);
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;  // Ctrl+D
        }
        
        // 检查退出命令
        if (strcmp(buffer, ".\n") == 0 || 
            strcmp(buffer, "exit\n") == 0 || 
            strcmp(buffer, "quit\n") == 0) {
            printf("exit xtee\n");
            break;
        }
        
        // 写入文件
        fprintf(fp, "%s", buffer);
        line_count++;
    }
    
    fclose(fp);
    printf("write completion rate: %d line",line_count); 
    printf("The content has been saved to %s\n", args[1]);
    return 0;
}

// 修复：添加(void)参数
int xhelp_cmd(char **args) {
    (void)args;  // 标记参数未使用
    printf("Built-in commands:\n");
    printf("------------------\n");
    for (int i = 0; builtins[i].name != NULL; i++) {
        printf("  %-10s - %s\n", builtins[i].name, builtins[i].description);
    }
    printf("\nExternal commands can also be executed.\n");
    printf("Use '|' for piping, '>' for output redirection, '<' for input redirection.\n");
    return 0;
}

// 修复：添加(void)参数
int xexit_cmd(char **args) {
    (void)args;  // 标记参数未使用
    printf("Exiting XShell...\n");
    exit(0);
    return 0;
}
//int xcat_cmd(char **args) {
//    
//  if (args[1] == NULL) {
//        // 从标准输入读取
//       char buffer[1024];
//      while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
//         printf("%s", buffer);
//     }
//      return 0;
//  }
//    
//  for (int i = 1; args[i] != NULL; i++) {
//      FILE *fp = fopen(args[i], "r");
//     if (fp == NULL) {
//          fprintf(stderr, "error: open failed '%s'\n", args[i]);
//          perror("xcat");
//          continue;
//      }
//      
//      printf("=== files: %s ===\n", args[i]);
//      
//      char buffer[1024];
//      int line_number = 0;
//      int has_content = 0;
//        
//      while (fgets(buffer, sizeof(buffer), fp) != NULL) {
//          has_content = 1;
//          line_number++;
//         printf("%4d: %s", line_number, buffer);
//      }
//        
//      if (!has_content) {
//          printf("(Null)\n");
//      }
//        
//     fclose(fp);
//      printf("=== files end ===\n\n");
//         }
//    
//  return 0;
//}
// 
//
//int xls_cmd(char **args) {
//
//    char *dir = ".";
//    if (args[1] != NULL) {
//      dir = args[1];
//    }
//    printf("Debug: xls attempting to open directory: %s\n", dir);
//    DIR *dp = opendir(dir);
//    if (dp == NULL) {
//    printf("Debug: opendir failed, will output error message\n"); 
////Ensure using stderr
//        fprintf(stderr, "xls error: Cannot access directory '%s'\n", dir);
//        perror("Reason");
//        
//        printf("Debug: Error message has been output\n");
//        return 1;
//    }
//    
//    struct dirent *entry;
//    while ((entry = readdir(dp)) != NULL) {
//        if (entry->d_name[0] != '.') {
//          char type = ' ';
//          struct stat st;
//          char fullpath[1024];
//          snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
//            
//            if (stat(fullpath, &st) == 0) {
//              if (S_ISDIR(st.st_mode)) type = 'd';
//              else if (S_ISLNK(st.st_mode)) type = 'l';
//              else if (S_ISREG(st.st_mode)) type = '-';
//          }
//            
//          printf("[%c] %s\n", type, entry->d_name);
//  }
//  }
//    
//   closedir(dp);
//  return 0;
//}