#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>   
#include <signal.h>
#include "execute.h"
#include "builtins.h" 
#include "log.h"
#include "jobs.h"

void execute_pipeline(Pipeline *pipeline) {
    if (pipeline == NULL || pipeline->num_commands == 0) return;

    int num_cmds = pipeline->num_commands;
    int pipes[20][2]; 
    
    // --- THE FIX: We track our exact children so we don't steal background jobs! ---
    pid_t pids[20];       
    int pid_count = 0;    

    // 1. Create all necessary pipes first
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe error");
            return;
        }
    }

    pid_t last_pid = -1; 

    // 2. Execute EVERY command in the pipeline
    for (int i = 0; i < num_cmds; i++) {
        Command *cmd = &pipeline->commands[i];

        if (cmd->argc == 0 || cmd->argv[0] == NULL) continue;

        // ---------------------------------------------------
        // THE BUILT-IN INTERCEPTOR
        // Built-ins skip fork(), so they NEVER get added to the pids[] array!
        // ---------------------------------------------------
        if (strcmp(cmd->argv[0], "hop") == 0) {
            execute_hop(cmd);
            continue; 
        }
        else if (strcmp(cmd->argv[0], "reveal") == 0) {
            execute_reveal(cmd);
            continue; 
        }
        else if (strcmp(cmd->argv[0], "log") == 0) { 
            execute_log(cmd);
            continue;
        }
        else if (strcmp(cmd->argv[0], "activities") == 0) { 
            print_activities();
            continue;
        }
        else if (strcmp(cmd->argv[0], "ping") == 0) { 
            execute_ping(cmd);
            continue;
        }
        else if (strcmp(cmd->argv[0], "fg") == 0) { // <--- ADD THIS
            execute_fg(cmd);
            continue;
        }
        else if (strcmp(cmd->argv[0], "bg") == 0) { // <--- ADD THIS
            execute_bg(cmd);
            continue;
        }
        // If it is NOT a built-in, we proceed with normal execution
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
        }
        else if (pid == 0) {
            // --- CHILD PROCESS ---
            // Unplug the ears so the child CAN be killed or paused!
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // A. Rewire the Pipes
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // B. Close all pipe FDs in the child
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // C. Handle File Redirection
            if (cmd->input_file != NULL) {
                int fd_in = open(cmd->input_file, O_RDONLY);
                if (fd_in < 0) {
                    printf("No such file or directory\n");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            if (cmd->output_file != NULL) {
                int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
                int fd_out = open(cmd->output_file, flags, 0644);
                if (fd_out < 0) {
                    printf("Unable to create file for writing\n");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            // D. Execute!
            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                printf("Command not found!\n");
                exit(1);
            }
        } 
        else {
            // --- PARENT PROCESS BLOCK INSIDE THE LOOP ---
            last_pid = pid;
            pids[pid_count] = pid; // Save the child to our array!
            pid_count++;           
        }
    }

    // --- PARENT PROCESS CLEANUP ---
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait logic (Sequential vs Background)
    if (pipeline->background) {
        if (last_pid > 0) {
            printf("[1] %d\n", last_pid); 
            add_job(last_pid, pipeline->commands[0].argv[0]); 
        }
    } else {
        // Foreground Execution
        // We ONLY loop through the exact PIDs we spawned. 
        for (int i = 0; i < pid_count; i++) {
            int status;
            pid_t wpid = waitpid(pids[i], &status, WUNTRACED);
            
            // Did Ctrl-Z pause it?
            if (wpid > 0 && WIFSTOPPED(status)) {
                printf("\n[%d] Stopped\n", wpid);
                add_job(wpid, pipeline->commands[0].argv[0]); 
                change_job_state(wpid, "Stopped");
            }
        }
    }
}