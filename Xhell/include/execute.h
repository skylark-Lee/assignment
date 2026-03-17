#ifndef EXECUTE_H
#define EXECUTE_H

#include "parser.h"

int execute(struct command *cmd);
int execute_external(struct command *cmd);

#endif