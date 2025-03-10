#ifndef TYPES_H
#define TYPES_H

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

#define BUILTIN_COUNT 5
#define NO_ARG_LIMIT -1

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

#endif /* TYPES_H */
