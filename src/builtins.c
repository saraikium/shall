#include "builtins.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// List of built-in commands with min & max argument constraints
const builtin_t builtin_commands[BUILTIN_COUNT] = {
    {"echo", 0, NO_ARG_LIMIT}, // echo requires at least 1 arg, no upper limit
    {"type", 0, NO_ARG_LIMIT}, // type requires exactly 1 argument
    {"pwd", 0, 0},             // pwd takes no arguments
    {"exit", 0, 1},            // exit can take 0 or 1 argument
    {"cd", 0, 1},              // cd takes 0 or 1 argument
};

int is_builtin(const command_t *cmd) {
  for (int i = 0; i < BUILTIN_COUNT; i++) {
    if (builtin_commands[i].name == cmd->name) {
      return 1;
    }
  }
  return 0;
}

const builtin_t *get_builtin(const char *cmd) {
  for (int i = 0; i < BUILTIN_COUNT; i++) {
    if (strcmp(builtin_commands[i].name, cmd) == 0) {
      return &builtin_commands[i];
    }
  }
  return NULL;
}

// Validate argument count (-1 = too few, 0 = valid, 1 = too many)
int validate_builtin_args(const command_t *cmd, const builtin_t *builtin) {
  int argc = cmd->argc - 1;
  if (argc < builtin->min_argc)
    return -1; // Too few arguments
  if (builtin->max_argc != -1 && argc > builtin->max_argc)
    return 1; // Too many arguments
  return 0;   // Valid argument count
}

void builtin_echo(const command_t *cmd) {
  size_t total_length = 0;
  // Calculate total length for buffer
  for (int i = 1; i < cmd->argc; i++) {
    total_length += strlen(cmd->argv[i]) + 1; // +1 for space/newline
  }

  // Allocate a single buffer to hold the full output
  char *buffer = malloc(total_length + 1); // +1 for null terminator
  if (!buffer) {
    perror("malloc");
    return;
  }

  buffer[0] = '\0';

  for (int i = 1; i < cmd->argc; i++) {
    strcat(buffer, cmd->argv[i]);
    if (i < cmd->argc - 1)
      strcat(buffer, " ");
  }
  strcat(buffer, "\n");
  write(STDOUT_FILENO, buffer, strlen(buffer));
  free(buffer);
}

void builtin_pwd() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd))) {
    printf("%s\n", cwd);
  } else {
    perror("pwd");
  }
}

void builtin_exit(const command_t *cmd) {
  int exit_code = 0;
  errno = 0;
  if (cmd->argc > 1) {
    exit_code = atoi(cmd->argv[1]);
  }
  if (errno != 0) {
    perror("atoi");
    exit(EXIT_FAILURE);
  }
  exit(exit_code);
}

void builtin_type(const command_t *cmd) {
  for (int i = 1; i < cmd->argc; i++) {
    // Should I free it? I'm confused here
    char *arg = cmd->argv[i];
    const builtin_t *builtin = get_builtin(arg);
    if (builtin != NULL) {
      printf("%s is a shell builtin\n", builtin->name);
    } else {
      const char *path = find_path(arg);
      if (path != NULL) {
        printf("%s is %s\n", arg, path);
      } else {
        printf("%s: not found\n", arg);
      }
    }
  }
  fflush(stdout);
}

void builtin_cd(const command_t *cmd) {
  char *path = cmd->argc > 1 ? cmd->argv[1] : getenv("HOME");

  if (path == NULL) {
    fprintf(stderr, "cd: Home not set\n");
    return;
  }
  if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
    char expanded_path[1024];
    const char *home = getenv("HOME");
    snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, path + 1);
    path = expanded_path;
  }
  if (chdir(path) != 0) {
    perror("cd");
  }
}

static void
run_builtin_with_redirection(const command_t *cmd,
                             void (*builtin_func)(const command_t *)) {
  int backup_stdin = -1;
  int backup_stdout = -1;

  // Backup original stdin/stdout
  backup_stdin = dup(STDIN_FILENO);
  backup_stdout = dup(STDOUT_FILENO);

  // Input redirection
  if (cmd->infile) {
    int fd_in = open(cmd->infile, O_RDONLY);
    if (fd_in < 0) {
      perror("open infile");
      return;
    }
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);
  }

  // Output redirection
  if (cmd->outfile) {
    int flags = O_WRONLY | O_CREAT;
    flags |= (cmd->append_out) ? O_APPEND : O_TRUNC;
    int fd_out = open(cmd->outfile, flags, 0644);
    if (fd_out < 0) {
      perror("open outfile");
      return;
    }
    dup2(fd_out, STDOUT_FILENO);
    close(fd_out);
  }

  // Now run the builtin with the new FDs
  builtin_func(cmd);

  // Restore original stdin/stdout
  dup2(backup_stdin, STDIN_FILENO);
  dup2(backup_stdout, STDOUT_FILENO);
  close(backup_stdin);
  close(backup_stdout);
}

int handle_builtin(const command_t *cmd) {
  const builtin_t *builtin = get_builtin(cmd->name);
  if (builtin == NULL)
    return 0;

  if (strcmp(cmd->name, "echo") == 0) {
    run_builtin_with_redirection(cmd, builtin_echo);
    return 1;
  } else if (strcmp(cmd->name, "pwd") == 0) {
    run_builtin_with_redirection(cmd, (void (*)(const command_t *))builtin_pwd);
    return 1;
  } else if (strcmp(cmd->name, "exit") == 0) {
    builtin_exit(cmd);
    return 1;
  } else if (strcmp(cmd->name, "type") == 0) {
    run_builtin_with_redirection(cmd, builtin_type);
    return 1;
  } else if (strcmp(cmd->name, "cd") == 0) {
    run_builtin_with_redirection(cmd, builtin_cd);
    return 1;
  }
  return 0; // Not handled
}
