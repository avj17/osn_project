#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

// A struct to hold a single parsed command
typedef struct {
    char *argv[100];      // Array of strings (command + arguments)
    int argc;             // Number of arguments
    char *input_file;     // NULL if no '<', otherwise the filename
    char *output_file;    // NULL if no '>', otherwise the filename
    bool append_output;   // true if '>>', false if '>'
    bool background;      // true if command ends with '&'
} Command;

// Takes the raw string, chops it up, and returns a populated Command struct
// Returns NULL if there is a syntax error
Command* parse_input(char *raw_input);

// Frees the memory allocated for the Command struct
void free_command(Command *cmd);

#endif