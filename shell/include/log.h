#ifndef LOG_H
#define LOG_H

#include "parser.h"

// Set up the file path when the shell boots
void init_log();

// Save a raw string to the file (max 15 commands)
void add_to_log(char *input);

// Run the log, log purge, or log execute commands
void execute_log(Command *cmd);

#endif