#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

void execute_hop(Command *cmd);
void execute_reveal(Command *cmd);
void execute_log(Command *cmd);
void execute_ping(Command *cmd);
void execute_fg(Command *cmd); // <--- Add this!
void execute_bg(Command *cmd); // <--- Add this!

#endif