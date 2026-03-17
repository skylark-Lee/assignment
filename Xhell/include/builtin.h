#ifndef BUILTIN_H
#define BUILTIN_H

typedef int (*builtin_func)(char **args);

struct builtin_command {
    char *name;
    builtin_func func;
    char *description;
};

int xpwd_cmd(char **args);
int xcd_cmd(char **args);
int xls_cmd(char **args);
int xtouch_cmd(char **args);
int xecho_cmd(char **args);
int xcat_cmd(char **args);
int xcp_cmd(char **args);
int xrm_cmd(char **args);
int xmv_cmd(char **args);
int xhistory_cmd(char **args);
int xtee_cmd(char **args);
int xexit_cmd(char **args);
int xhelp_cmd(char **args);
int xfind_cmd(char **args);
int xwc_cmd(char **args);

builtin_func get_builtin(char *name);

#endif