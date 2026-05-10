#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>         //For the username
#include <sys/types.h>
#include "prompt.h"
#include "globals.h"

void display_prompt() {
    char hostname[256];
    char cwd[PATH_MAX];
    char *username;

    // 1. Get the username
    // We get the user ID, then look up the password struct to get the name
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL) {
        username = pw->pw_name;
    } else {
        username = "unknown";
    }

    // 2. Get the system/host name
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }

    // 3. Get the Current Working Directory (CWD)
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd error");
        return;
    }

    // 4. Handle the '~' replacement logic
    char display_path[PATH_MAX];
    int home_len = strlen(home_dir);

    // Check if the current directory starts with the home directory
    if (strncmp(cwd, home_dir, home_len) == 0) {
        // If the cwd is EXACTLY the home dir, or a subfolder of it
        // We copy '~' into display_path, then append the rest of the cwd
        snprintf(display_path, sizeof(display_path), "~%s", cwd + home_len);
    } else {
        // If we hopped outside the home directory, just show the absolute path
        strncpy(display_path, cwd, sizeof(display_path));
    }

    // 5. Print the prompt
    printf("<%s@%s:%s> ", username, hostname, display_path);
    
    // CRITICAL: Because our prompt does not have a newline (\n) at the end,
    // C might buffer it and not show it immediately. fflush forces it to print.
    fflush(stdout); 
}