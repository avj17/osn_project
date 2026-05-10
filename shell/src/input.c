#include <stdio.h>
#include <stdlib.h>
#include "input.h"

char* read_user_input() {
    char *line = NULL;
    size_t bufsize = 0; // getline will automatically allocate memory if we set this to 0

    // getline reads from standard input (your keyboard) until you press Enter
    if (getline(&line, &bufsize, stdin) == -1) {
        // If getline returns -1, it usually means it hit an EOF (End of File)
        // This happens when you press Ctrl+D. We should exit gracefully.
        if (feof(stdin)) {
            printf("\nlogout\n");
            exit(0);
        } else {
            // A real error occurred
            perror("Error reading input");
            exit(1);
        }
    }

    return line;
}