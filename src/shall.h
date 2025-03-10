#ifndef SHALL_H
#define SHALL_H

// Ensure POSIX functions like strdup() are available
#define _POSIX_C_SOURCE 200809L
#define __GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_input(const char *input);

#define BUILTIN_COUNT 4

typedef struct {
  int argc;
  char *name;
  char **argv;
} command_t;

typedef struct {
  char *name;   // Command name
  int min_argc; // Minimum required arguments
  int max_argc; // Maximum allowed arguments (-1 means no limit)
} builtin_t;

void handle_input(const char *input);
void clean_input(char *input, int buffer_size);

#endif /* SHALL_H */
