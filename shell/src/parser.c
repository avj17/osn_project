#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

Command* parse_input(char *raw_input) {
    // If input is empty, do nothing
    if (raw_input == NULL || strlen(raw_input) == 0) {
        return NULL;
    }

    // Allocate memory for our new command struct
    Command *cmd = malloc(sizeof(Command));
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = false;
    cmd->background = false;

    // We need to chop the raw_input string based on spaces, tabs, and newlines
    const char delimiters[] = " \t\r\n\a";
    
    // strtok is the standard C function for splitting strings
    char *token = strtok(raw_input, delimiters);

    while (token != NULL) {
        // For now, let's just assume everything is an argument
        // We will add the logic for <, >, &, and | later
        cmd->argv[cmd->argc] = strdup(token); // Copy the string safely
        cmd->argc++;

        // Get the next piece of the string
        token = strtok(NULL, delimiters);
    }

    // execvp requires the argument array to be NULL-terminated!
    cmd->argv[cmd->argc] = NULL;

    // If they just pressed Enter without typing a command, free and return NULL
    if (cmd->argc == 0) {
        free(cmd);
        return NULL;
    }

    return cmd;
}

void free_command(Command *cmd) {
    if (cmd == NULL) return;
    
    // Free all the copied argument strings
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    
    // Free the struct 
    free(cmd);
}