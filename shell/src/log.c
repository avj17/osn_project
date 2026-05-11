#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "globals.h"
#include "parser.h"
#include "execute.h"

char log_path[PATH_MAX + 64]; // Give it extra padding so snprintf never truncates

void init_log() {
    // Store the history file directly inside the shell's home directory
    snprintf(log_path, sizeof(log_path), "%s/.shell_history.txt", home_dir);
}

void add_to_log(char *input) {
    // 1. Requirement: Do not store any command if it contains 'log'
    if (strstr(input, "log") != NULL) return;

    // Remove the trailing newline character for clean storage
    char clean_input[1024];
    strcpy(clean_input, input);
    clean_input[strcspn(clean_input, "\r\n")] = 0;
    
    if (strlen(clean_input) == 0) return;

    // 2. Read the existing log file into an array
    FILE *f = fopen(log_path, "r");
    char history[15][1024];
    int count = 0;
    
    if (f != NULL) {
        while (fgets(history[count], 1024, f)) {
            history[count][strcspn(history[count], "\r\n")] = 0; 
            count++;
        }
        fclose(f);
    }

    // 3. Requirement: Do not store if identical to the previously executed command
    if (count > 0 && strcmp(history[count - 1], clean_input) == 0) return; 

    // 4. Add the new command (shift up if we already have 15)
    if (count == 15) {
        for (int i = 0; i < 14; i++) {
            strcpy(history[i], history[i+1]);
        }
        strcpy(history[14], clean_input);
    } else {
        strcpy(history[count], clean_input);
        count++;
    }

    // 5. Write the updated array back to the file
    f = fopen(log_path, "w");
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", history[i]);
    }
    fclose(f);
}

void execute_log(Command *cmd) {
    if (cmd->argc == 1) {
        // 'log': Print history oldest to newest
        FILE *f = fopen(log_path, "r");
        if (!f) return;
        char line[1024];
        while (fgets(line, 1024, f)) {
            printf("%s", line);
        }
        fclose(f);
    }
    else if (cmd->argc == 2 && strcmp(cmd->argv[1], "purge") == 0) {
        // 'log purge': Delete the history file
        remove(log_path);
    }
    else if (cmd->argc == 3 && strcmp(cmd->argv[1], "execute") == 0) {
        // 'log execute <index>'
        int index = atoi(cmd->argv[2]);
        
        FILE *f = fopen(log_path, "r");
        if (!f) return;
        char history[15][1024];
        int count = 0;
        while (fgets(history[count], 1024, f)) {
            history[count][strcspn(history[count], "\r\n")] = 0;
            count++;
        }
        fclose(f);

        if (index <= 0 || index > count) {
            printf("log: Invalid index\n");
            return;
        }

        // 1-indexed from NEWEST to OLDEST
        char *target_cmd = history[count - index];
        
        // We act like the Traffic Cop from main.c and run this string!
        char *start = target_cmd;
        int len = strlen(target_cmd);
        for (int i = 0; i <= len; i++) {
            if (target_cmd[i] == ';' || target_cmd[i] == '&' || target_cmd[i] == '\0') {
                char delim = target_cmd[i];
                target_cmd[i] = '\0';
                Pipeline *pipeline = parse_input(start);
                if (pipeline != NULL) {
                    if (delim == '&') pipeline->background = true;
                    execute_pipeline(pipeline);
                    free_pipeline(pipeline);
                }
                start = &target_cmd[i + 1];
            }
        }
    }
    else {
        printf("log: Invalid Syntax!\n");
    }
}