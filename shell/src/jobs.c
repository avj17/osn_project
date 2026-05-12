#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"
#include <signal.h>


Job job_table[100];

void init_jobs() {
    for (int i = 0; i < 100; i++) {
        job_table[i].active = false;
    }
}

void add_job(pid_t pid, char *command_name) {
    for (int i = 0; i < 100; i++) {
        if (!job_table[i].active) {
            job_table[i].pid = pid;
            strcpy(job_table[i].command_name, command_name);
            strcpy(job_table[i].state, "Running"); // Jobs start as Running by default
            job_table[i].active = true;
            return;
        }
    }
    printf("Job table is full!\n");
}

void remove_job(pid_t pid) {
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active && job_table[i].pid == pid) {
            job_table[i].active = false;
            return;
        }
    }
}

// Helper function to sort jobs alphabetically for the activities command
int compare_jobs(const void *a, const void *b) {
    Job *jobA = (Job *)a;
    Job *jobB = (Job *)b;
    return strcmp(jobA->command_name, jobB->command_name);
}



void print_activities() {
    Job active_jobs[100];
    int count = 0;
    
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active) {
            
            // THE ZOMBIE KILLER: Read the Linux kernel's exact process state
            char path[256];
            snprintf(path, sizeof(path), "/proc/%d/stat", job_table[i].pid);
            FILE *f = fopen(path, "r");

            if (f != NULL) {
                char line[1024];
                if (fgets(line, sizeof(line), f)) {
                    // The file format is: PID (Command) State
                    // We find the last parenthesis to safely skip the command name
                    char *rparen = strrchr(line, ')');
                    if (rparen != NULL) {
                        char state = *(rparen + 2); // State is 2 characters after ')'

                        // 'Z' = Zombie, 'X' = Dead
                        if (state == 'Z' || state == 'X') {
                            fclose(f);
                            continue; // Skip it! The Reaper will clean it up later.
                        }

                        // Bonus: Dynamically update Stopped vs Running
                        if (state == 'T') {
                            strcpy(job_table[i].state, "Stopped");
                        } else {
                            strcpy(job_table[i].state, "Running");
                        }
                    }
                }
                fclose(f);
                
                // If it wasn't a zombie, add it to our print list
                active_jobs[count] = job_table[i];
                count++;
            } else {
                // If the file doesn't exist at all, the process is completely gone
                job_table[i].active = false;
            }
        }
    }

    qsort(active_jobs, count, sizeof(Job), compare_jobs);

    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", active_jobs[i].pid, active_jobs[i].command_name, active_jobs[i].state);
    }
}


// Update the state of a job (e.g., from Running to Stopped)
void change_job_state(pid_t pid, const char *new_state) {
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active && job_table[i].pid == pid) {
            strcpy(job_table[i].state, new_state);
            return;
        }
    }
}