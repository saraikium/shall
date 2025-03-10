#include "shall.h"

// List of built-in commands with min & max argument constraints
const builtin_t builtin_commands[BUILTIN_COUNT] = {
    {"echo", 0, -1}, // echo requires at least 1 arg, no upper limit
    {"type", 0, -1}, // type requires exactly 1 argument
    {"pwd", 0, 0},   // pwd takes no arguments
    {"exit", 0, 1},  // exit can take 0 or 1 argument
};

const builtin_t *get_builtin(const char *cmd) {
  for (int i = 0; i < BUILTIN_COUNT; i++) {
    if (strcmp(builtin_commands[i].name, cmd) == 0) {
      return &builtin_commands[i];
    }
  }
  return NULL;
}

// Validate argument count (-1 = too few, 0 = valid, 1 = too many)
int validate_builtin_args(const builtin_t *builtin, int argc) {
  if (argc < builtin->min_argc)
    return -1; // Too few arguments
  if (builtin->max_argc != -1 && argc > builtin->max_argc)
    return 1; // Too many arguments
  return 0;   // Valid argument count
}

command_t *parse_command(const char *input) {
  // Note: Make a copy of input because strtok modifies it,
  // fucking C Shenanegans
  char *input_copy = strdup(input);
  if (!input_copy) {
    perror("strdup failed");
    return NULL;
  }

  // First, count the number of arguments
  int arg_count = 0;
  char *token = strtok(input_copy, " ");
  while (token) {
    arg_count++;
    token = strtok(NULL, " ");
  }

  // Allocate memory for command_t
  command_t *cmd = malloc(sizeof(command_t));
  if (!cmd) {
    perror("memory allocation failed");
    free(input_copy);
    return NULL;
  }

  cmd->argc = arg_count;
  // Allocate memory for argv (one extra for NULL termination)
  cmd->argv = malloc((arg_count + 1) * sizeof(char *));
  if (!cmd->argv) {
    perror("malloc failed");
    free(cmd);
    free(input_copy);
    return NULL;
  }

  // Tokenize again and store arguments
  strcpy(input_copy, input); // Restore original string
  token = strtok(input_copy, " ");
  int i = 0;
  while (token) {
    cmd->argv[i++] = strdup(token);
    token = strtok(NULL, " ");
  }
  cmd->argv[i] = NULL; // NULL-terminate argv

  // Set the command name (first argument)
  cmd->name = cmd->argv[0];

  free(input_copy);
  return cmd;
}

// Cleanup function to free allocated memory
void free_command(command_t *cmd) {
  if (!cmd)
    return;
  for (int i = 0; cmd->argv[i] != NULL; i++) {
    free(cmd->argv[i]);
  }
  free(cmd->argv);
  free(cmd);
}

// Check if the file exists and is executable
int is_executable(const char *path) { return access(path, X_OK); }

char *find_in_path(const char *command) {
  char *path_env = getenv("PATH");
  if (path_env == NULL) {
    return NULL;
  }
  char *path_copy = strdup(path_env);
  char *dir = strtok(path_copy, ":");
  static char full_path[1024];

  while (dir != NULL) {
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
    if (is_executable(full_path) == 0) {
      free(path_copy);
      return full_path;
    }

    dir = strtok(NULL, ":");
  }

  free(path_copy);
  return NULL;
}

void fork_and_exec_cmd(char *full_path, char **argv) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
  }

  if (pid == 0) {
    execv(full_path, argv);
    perror("exev");
    exit(EXIT_FAILURE);
  } else {
    int status;
    waitpid(pid, &status, 0);
  }
}

void handle_builtin(const command_t *cmd) {
  if (strcmp(cmd->name, "exit") == 0) {
    errno = 0;
    char *endptr;
    int exit_status = strtol(cmd->argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
      perror("strtol");
      exit(EXIT_FAILURE);
    }
    exit(exit_status);
  } else if (strcmp(cmd->name, "echo") == 0) {
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
  } else if (strcmp(cmd->name, "type") == 0) {
    for (int i = 1; i < cmd->argc; i++) {
      // Should I free it? I'm confused here
      char *arg = cmd->argv[i];
      const builtin_t *builtin = get_builtin(arg);
      if (builtin != NULL) {
        printf("%s is a shell builtin\n", builtin->name);
      } else {
        const char *path = find_in_path(arg);
        if (path != NULL) {
          printf("%s is %s\n", arg, path);
        } else {
          printf("%s: not found\n", arg);
        }
      }
    }
    fflush(stdout);
  } else if (strcmp(cmd->name, "pwd") == 0) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("%s\n", cwd);
    } else {
      perror("pwd");
    }
  }
}

void handle_input(const char *input) {
  // Handle the exit command
  command_t *cmd = parse_command(input);
  if (cmd == NULL) {
    return;
  }

  const builtin_t *builtin_cmd = get_builtin(cmd->name);

  if (builtin_cmd != NULL) {
    int args_valid = validate_builtin_args(builtin_cmd, cmd->argc - 1);
    if (args_valid < 0) {
      fprintf(stderr, "%s: too few arguments\n", cmd->name);
      return;
    }

    if (args_valid > 0) {
      fprintf(stderr, "%s: too many arguments\n", cmd->name);
      return;
    }
    handle_builtin(cmd);
    return;

  } else {

    char *command_path = find_in_path(cmd->name);
    if (command_path) {
      fork_and_exec_cmd(command_path, cmd->argv);
    } else {
      fprintf(stderr, "%s: command not found\n", input);
    }
  }
}

void clean_input(char *input, int buffer_size) {
  if (fgets(input, buffer_size, stdin) != NULL) {
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
      input[len - 1] = '\0';
    }
  }
}
