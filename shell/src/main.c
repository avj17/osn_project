#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "execute.h" 
#include "globals.h"
#include "log.h"
#include "jobs.h"
#include <signal.h>

// Initialize Global Variables
char home_dir[PATH_MAX];
char prev_dir[PATH_MAX] = ""; 

int main() {
    // 1. Boot up: Set the home directory
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror("Failed to get the starting directory");
        exit(1); 
    }

    // 2. Boot up: Initialize the log file path
    init_log();

    // 3. Boot up: Initialize the jobs table
    init_jobs();
    signal(SIGINT, SIG_IGN);  // Ignore Ctrl-C
    signal(SIGTSTP, SIG_IGN); // Ignore Ctrl-Z

    // 4. The Infinite Loop
    while (1) {
        // --- STEP 1: THE REAPER ---
        int status;
        pid_t bg_pid;
        while ((bg_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            // Remove the dead job from our registry!
            remove_job(bg_pid); 
            
            if (WIFEXITED(status)) {
                printf("Background process with pid %d exited normally\n", bg_pid);
            } else {
                printf("Background process with pid %d exited abnormally\n", bg_pid);
            }
        }

        // --- STEP 2: THE PROMPT ---
        display_prompt();

        // --- STEP 3: READ INPUT ---
        char *input = read_user_input();

        // --- STEP 4: SAVE TO LOG ---
        // Save the raw string to history before the Traffic Cop chops it up!
        add_to_log(input); 

        // --- STEP 5: THE TRAFFIC COP ---
        char *start = input;
        int len = strlen(input);
        
        for (int i = 0; i <= len; i++) {
            // If we hit a delimiter, OR we reach the very end of the string
            if (input[i] == ';' || input[i] == '&' || input[i] == '\0') {
                
                char delim = input[i]; // Remember which symbol triggered the cut
                input[i] = '\0';       // Terminate the string here to slice it

                // Parse the sliced chunk
                Pipeline *pipeline = parse_input(start);

                // Execute the chunk
                if (pipeline != NULL) {
                    if (delim == '&') {
                        pipeline->background = true;
                    }
                    
                    execute_pipeline(pipeline);
                    free_pipeline(pipeline);
                }
                
                // Move our starting pointer to the character immediately after the delimiter
                start = &input[i + 1];
            }
        }

        // Clean up the memory!
        free(input); 
    }

    return 0;
}