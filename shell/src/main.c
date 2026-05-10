#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "prompt.h"
#include "input.h"
#include "parser.h" // <-- Add parser header
#include "globals.h"

char home_dir[PATH_MAX];

int main() {
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror("Failed to get the starting directory");
        exit(1); 
    }

    while (1) {
        // Step A: Print the interface
        display_prompt();

        // Step B: Read user input
        char *input = read_user_input();

        // Step C: Parse the input
        Command *cmd = parse_input(input);

        // --- TEMPORARY PRINT BLOCK TO PROVE IT WORKS ---
        if (cmd != NULL) {
            printf("PARSER OUTPUT:\n");
            printf("  Command: %s\n", cmd->argv[0]);
            for (int i = 1; i < cmd->argc; i++) {
                printf("  Arg %d: %s\n", i, cmd->argv[i]);
            }
            // Free the command when we are done looking at it
            free_command(cmd);
        }
        // -----------------------------------------------

        // Step D: Execute (Coming next...)

        free(input); 
    }

    return 0;
}