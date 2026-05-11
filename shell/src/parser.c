#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

Pipeline* parse_input(char *raw_input) {
    if (raw_input == NULL || strlen(raw_input) == 0) return NULL;

    Pipeline *pipeline = malloc(sizeof(Pipeline));
    pipeline->num_commands = 1;
    pipeline->background = false;

    // Point to the first command in our array
    Command *curr_cmd = &pipeline->commands[0];
    curr_cmd->argc = 0;
    curr_cmd->input_file = NULL;
    curr_cmd->output_file = NULL;
    curr_cmd->append_output = false;

    const char delimiters[] = " \t\r\n\a";
    char *token = strtok(raw_input, delimiters);

    while (token != NULL) {
        if (strcmp(token, "|") == 0) {
            // Cap off the current command
            curr_cmd->argv[curr_cmd->argc] = NULL; 
            
            // Move to the next command in the array
            pipeline->num_commands++;
            curr_cmd = &pipeline->commands[pipeline->num_commands - 1];
            
            // Initialize the new command
            curr_cmd->argc = 0;
            curr_cmd->input_file = NULL;
            curr_cmd->output_file = NULL;
            curr_cmd->append_output = false;
        } 
        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, delimiters);
            if (token != NULL) curr_cmd->input_file = strdup(token);
        } 
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, delimiters);
            if (token != NULL) {
                curr_cmd->output_file = strdup(token);
                curr_cmd->append_output = false;
            }
        } 
        else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, delimiters);
            if (token != NULL) {
                curr_cmd->output_file = strdup(token);
                curr_cmd->append_output = true;
            }
        } 
        

        
        else {
            curr_cmd->argv[curr_cmd->argc] = strdup(token);
            curr_cmd->argc++;
        }
        token = strtok(NULL, delimiters);
    }

    // Cap off the very last command in the pipeline
    curr_cmd->argv[curr_cmd->argc] = NULL;

    if (pipeline->commands[0].argc == 0) {
        free_pipeline(pipeline);
        return NULL;
    }

    return pipeline;
}

void free_pipeline(Pipeline *pipeline) {
    if (pipeline == NULL) return;
    
    // Loop through every command and free its strings
    for (int i = 0; i < pipeline->num_commands; i++) {
        Command *cmd = &pipeline->commands[i];
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
        if (cmd->input_file != NULL) free(cmd->input_file);
        if (cmd->output_file != NULL) free(cmd->output_file);
    }
    free(pipeline);
}