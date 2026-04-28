#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

//This tells the compiler "this variable exists somewhere else in the .c files"
extern char home_dir[PATH_MAX];

#endif