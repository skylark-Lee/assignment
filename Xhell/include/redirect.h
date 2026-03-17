#ifndef REDIRECT_H
#define REDIRECT_H

#include "parser.h"

int setup_redirection(struct command *cmd);
int restore_redirection(void);

#endif