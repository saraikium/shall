#ifndef TYPES_H
#define TYPES_H

// Ensure POSIX functions like strdup() are available
#define _POSIX_C_SOURCE 200809L
#define __GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
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
  // IO redirection info
  char *infile;
  char *outfile;
  int append_out; // 1 for appending 0 for overwriting
} command_t;

typedef struct {
  char *name;   // Command name
  int min_argc; // Minimum required arguments
  int max_argc; // Maximum allowed arguments (-1 means no limit)
} builtin_t;

// Token types for parsing
typedef enum {
  TOKEN_WORD,         // Regular command/argument
  TOKEN_PIPE,         // |
  TOKEN_REDIR_IN,     // <
  TOKEN_REDIR_OUT,    // >
  TOKEN_REDIR_APPEND, // >>
  TOKEN_BACKGROUND,   // &
  TOKEN_EOF,          // End of input
  TOKEN_ERROR         // Invalid token
} token_type_t;

// Token structure
typedef struct {
  token_type_t type;
  char *value;
} token_t;

#endif /* TYPES_H */
