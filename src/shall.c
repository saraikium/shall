#include "shall.h"
#include "builtins.h"
#include "parser.h"
#include "shall.h"
#include "utils.h"

void clean_input(char *input, int buffer_size) {
  if (fgets(input, buffer_size, stdin) != NULL) {
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
      input[len - 1] = '\0';
    }
  }
}

// Cleanup function to free allocated memory
void free_command(command_t *cmd) {
  if (cmd == NULL)
    return;
  // Free argv array safely
  if (cmd->argv) {
    for (int i = 0; cmd->argv[i] != NULL; i++) {
      free(cmd->argv[i]);
      cmd->argv[i] = NULL; // Prevent dangling pointer
    }
    free(cmd->argv);
    cmd->argv = NULL;
  }

  // Free infile if it was allocated
  if (cmd->infile) {
    free(cmd->infile);
    cmd->infile = NULL;
  }

  // Free outfile if it was allocated
  if (cmd->outfile) {
    free(cmd->outfile);
    cmd->outfile = NULL;
  }

  free(cmd);  // Free the struct itself
  cmd = NULL; // Prevent accidental use after free
}

void handle_input(const char *input) {
  // Step 1: tokenize the input
  int num_tokens = 0;
  token_t *tokens = tokenize_input(input, &num_tokens);
  if (!tokens)
    return;

  // Step 2: Parse the command from tokens
  command_t *cmd = parse_command(tokens, num_tokens);
  if (!cmd) {
    fprintf(stderr, "Error parsing command\n");
    return;
  }

  // Step3: Handle execution
  // Check if cmd is a builtin, if it is handle it and return 1 else return 0
  int builtin_handled = handle_builtin(cmd);

  if (builtin_handled)
    return;

  char *command_path = find_path(cmd->name);
  if (command_path) {
    fork_and_exec_cmd(cmd, command_path);
  } else {
    fprintf(stderr, "%s: command not found\n", input);
  }

  // Free tokens
  free_tokens(tokens, num_tokens);
  // Free command structure
  free_command(cmd);
}
