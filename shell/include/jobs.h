#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include <stdbool.h>

// Represents a single background or stopped process
typedef struct {
    pid_t pid;
    char command_name[1024]; // What the user typed (e.g., "sleep 10")
    char state[20];          // "Running" or "Stopped"
    bool active;             // true if this slot is taken, false if empty
} Job;

extern Job job_table[100]; // We will support up to 100 jobs at a time

// Initialize the table on boot
void init_jobs();

// Add a new job to the table
void add_job(pid_t pid, char *command_name);

// Remove a job when the Reaper catches it dying
void remove_job(pid_t pid);

// Requirement E.1: Print all active jobs, sorted lexicographically
void print_activities();

#endif