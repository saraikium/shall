#include "shall.h"
#include "builtins.h"
#include "utils.h"

void clean_input(char *input, int buffer_size) {
  if (fgets(input, buffer_size, stdin) != NULL) {
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
      input[len - 1] = '\0';
    }
  }
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

void handle_input(const char *input) {
  // Handle the exit command
  command_t *cmd = parse_command(input);
  if (cmd == NULL) {
    return;
  }

  // Check if cmd is a builtin, if it is handle it and return 1 else return 0
  int builtin_handled = handle_builtin(cmd);

  if (builtin_handled)
    return;

  char *command_path = find_path(cmd->name);
  if (command_path) {
    fork_and_exec_cmd(command_path, cmd->argv);
  } else {
    fprintf(stderr, "%s: command not found\n", input);
  }
}
