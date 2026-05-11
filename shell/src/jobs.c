#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"

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
    // 1. Gather all active jobs into a temporary array so we can sort them
    Job active_jobs[100];
    int count = 0;
    
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active) {
            active_jobs[count] = job_table[i];
            count++;
        }
    }

    // 2. Sort them lexicographically
    qsort(active_jobs, count, sizeof(Job), compare_jobs);

    // 3. Print them exactly as the assignment requires
    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", active_jobs[i].pid, active_jobs[i].command_name, active_jobs[i].state);
    }
}