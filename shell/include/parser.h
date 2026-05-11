#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

// A single command (e.g., "ls -l > out.txt")
typedef struct {
    char *argv[100];
    int argc;
    char *input_file;
    char *output_file;
    bool append_output;
} Command;

// A full pipeline (e.g., "ls -l | wc | grep 5")
typedef struct {
    Command commands[20]; // We support up to 20 commands chained together
    int num_commands;
    bool background;      // true if it ends with '&'
} Pipeline;

Pipeline* parse_input(char *raw_input);
void free_pipeline(Pipeline *pipeline);

#endif