#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h> // Required for opendir, readdir, and closedir
#include "builtins.h"
#include "globals.h"
#include <signal.h>
#include <sys/wait.h> // Required for waitpid
#include <signal.h>   // Required for kill and SIGCONT
#include "jobs.h"     // Required to read and edit the job_table


// Helper function for qsort to sort strings alphabetically
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// ---------------------------------------------------------
// REQUIREMENT B.1: hop (Change Directory)
// ---------------------------------------------------------
void execute_hop(Command *cmd) {
    char current_dir[PATH_MAX];
    char target_dir[PATH_MAX];

    // If no arguments, jump to home
    if (cmd->argc == 1) {
        getcwd(current_dir, sizeof(current_dir));
        if (chdir(home_dir) == 0) {
            strcpy(prev_dir, current_dir);
        } else {
            perror("hop");
        }
        return;
    }

    // Loop through every argument sequentially (e.g., hop dir1 dir2 ..)
    for (int i = 1; i < cmd->argc; i++) {
        // Always grab our current location before we move, so we can save it to prev_dir
        getcwd(current_dir, sizeof(current_dir));
        
        char *arg = cmd->argv[i];

        if (strcmp(arg, "~") == 0) {
            strcpy(target_dir, home_dir);
        } 
        else if (strcmp(arg, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                // Do nothing if there is no previous directory yet
                continue; 
            }
            strcpy(target_dir, prev_dir);
        } 
        else {
            // This handles ".", "..", and absolute/relative paths natively
            strcpy(target_dir, arg);
        }

        // Attempt to change the directory
        if (chdir(target_dir) != 0) {
            printf("No such directory!\n");
        } else {
            // If successful, the old current_dir is now our prev_dir
            strcpy(prev_dir, current_dir);
        }
    }
}

// ---------------------------------------------------------
// REQUIREMENT B.2: reveal (List Directory Contents)
// ---------------------------------------------------------
void execute_reveal(Command *cmd) {
    bool flag_a = false;
    bool flag_l = false;
    char target_dir[PATH_MAX] = "."; // Default is the current directory
    int path_count = 0;

    // 1. Parse the arguments for flags and the target path
    for (int i = 1; i < cmd->argc; i++) {
        char *arg = cmd->argv[i];
        
        // If it starts with '-' and isn't exactly the hop "previous dir" command
        if (arg[0] == '-' && strlen(arg) > 1 && strcmp(arg, "-") != 0) {
            for (int j = 1; arg[j] != '\0'; j++) {
                if (arg[j] == 'a') flag_a = true;
                else if (arg[j] == 'l') flag_l = true;
            }
        } 
        else {
            // It's a path argument
            path_count++;
            if (path_count > 1) {
                printf("reveal: Invalid Syntax!\n");
                return;
            }

            // Path translation logic (same as hop)
            if (strcmp(arg, "~") == 0) {
                strcpy(target_dir, home_dir);
            } else if (strcmp(arg, "-") == 0) {
                if (strlen(prev_dir) == 0) {
                    printf("No such directory!\n");
                    return;
                }
                strcpy(target_dir, prev_dir);
            } else {
                strcpy(target_dir, arg);
            }
        }
    }

    // 2. Open the directory
    DIR *dir = opendir(target_dir);
    if (dir == NULL) {
        printf("No such directory!\n");
        return;
    }

    // 3. Read the entries into an array so we can sort them
    struct dirent *entry;
    char *files[4096]; // Assume a max of 4096 files in a folder for this project
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // If the -a flag is NOT set, skip hidden files (files starting with '.')
        if (!flag_a && entry->d_name[0] == '.') {
            continue;
        }
        files[file_count] = strdup(entry->d_name);
        file_count++;
    }
    closedir(dir);

    // 4. Sort the files lexicographically (Requirement)
    qsort(files, file_count, sizeof(char *), compare_strings);

    // 5. Print the files based on the -l flag
    for (int i = 0; i < file_count; i++) {
        if (flag_l) {
            // Print one per line
            printf("%s\n", files[i]);
        } else {
            // Print space-separated
            printf("%s ", files[i]);
        }
        free(files[i]); // Clean up the memory we allocated with strdup
    }

    // If we printed space-separated, add a newline at the very end
    if (!flag_l && file_count > 0) {
        printf("\n");
    }
}



// ---------------------------------------------------------
// REQUIREMENT E.2: ping (Send signals to processes)
// ---------------------------------------------------------
void execute_ping(Command *cmd) {
    // Syntax check: ping <pid> <signal_number>
    if (cmd->argc != 3) {
        printf("Usage: ping <pid> <signal_number>\n");
        return;
    }

    // Convert the string arguments into integers
    pid_t target_pid = atoi(cmd->argv[1]);
    int signal_number = atoi(cmd->argv[2]);

    // Many assignments require you to take the signal modulo 32 to handle 
    // edge cases where a user types a massive number or negative number.
    // E.g., if they type ping 1234 9, sig becomes 9.
    int sig = signal_number % 32;

    // Send the signal!
    if (kill(target_pid, sig) == 0) {
        printf("Sent signal %d to process with pid %d\n", sig, target_pid);
    } else {
        // If it fails (e.g., process doesn't exist, or no permission)
        perror("No such process found");
    }
}




// ---------------------------------------------------------
// REQUIREMENT E.4: bg (Resume a stopped background job)
// ---------------------------------------------------------
void execute_bg(Command *cmd) {
    if (cmd->argc != 2) {
        printf("Usage: bg <pid>\n");
        return;
    }
    pid_t target_pid = atoi(cmd->argv[1]);

    bool found = false;
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active && job_table[i].pid == target_pid) {
            found = true;
            strcpy(job_table[i].state, "Running"); // Update state
            break;
        }
    }
    if (!found) {
        printf("No such process found\n");
        return;
    }

    // Wake it up! SIGCONT tells a paused process to continue running.
    kill(target_pid, SIGCONT);
}

// ---------------------------------------------------------
// REQUIREMENT E.4: fg (Bring a background job to the foreground)
// ---------------------------------------------------------
void execute_fg(Command *cmd) {
    if (cmd->argc != 2) {
        printf("Usage: fg <pid>\n");
        return;
    }
    pid_t target_pid = atoi(cmd->argv[1]);

    char cmd_name[1024] = "";
    bool found = false;

    // 1. Find the job and REMOVE it from the background table
    for (int i = 0; i < 100; i++) {
        if (job_table[i].active && job_table[i].pid == target_pid) {
            found = true;
            strcpy(cmd_name, job_table[i].command_name);
            job_table[i].active = false; // It is no longer in the background!
            break;
        }
    }
    
    if (!found) {
        printf("No such process found\n");
        return;
    }

    // 2. Wake it up in case it was stopped by Ctrl-Z
    kill(target_pid, SIGCONT);

    // 3. Wait for it, exactly like we do in execute.c for normal foreground jobs!
    // We must ignore keyboard signals in the parent shell while we wait.
    int status;
    pid_t wpid = waitpid(target_pid, &status, WUNTRACED);

    // 4. Did the user press Ctrl-Z while it was in the foreground?
    if (wpid > 0 && WIFSTOPPED(status)) {
        printf("\n[%d] Stopped\n", wpid);
        // Put it BACK into the background table!
        add_job(wpid, cmd_name); 
        change_job_state(wpid, "Stopped");
    }
}