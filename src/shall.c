#include "shall.h"
#include "builtins.h"
#include "parser.h"
#include "shall.h"
#include "utils.h"

static pid_t g_child_pids[MAX_CMDS];
static int g_pid_count = 0;

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

void free_commands(command_t **cmd_list, int num_commands) {
  if (!cmd_list)
    return;

  for (int i = 0; i < num_commands; i++) {
    if (cmd_list[i]) {
      free_command(cmd_list[i]);
    }
  }
  free(cmd_list);
}

static void run_builtin(const command_t *cmd) {
  if (cmd->background == 0) {
    handle_builtin(cmd);
    return;
  }
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return;
  }
  if (pid == 0) {
    // No need to handle redirection here. Redirection is already handled.
    handle_builtin(cmd);
    exit(EXIT_SUCCESS);
  } else {
    g_child_pids[g_pid_count++] = pid;
  }
}

void run_external_cmd(const command_t *cmd) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    if (cmd->infile) {
      int fdin = open(cmd->infile, O_RDONLY);
      if (fdin < 0) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      dup2(fdin, STDIN_FILENO);
      close(fdin);
    }

    if (cmd->outfile) {
      int flags = O_RDONLY | O_CREAT | (cmd->append_out ? O_APPEND : O_TRUNC);
      int fdout = open(cmd->outfile, flags);
      if (fdout < 0) {
        perror("Outfile");
        exit(EXIT_FAILURE);
      }
      dup2(fdout, STDOUT_FILENO);
      close(fdout);
    }

    char *path = find_path(cmd->name);
    if (!path) {
      fprintf(stderr, "%s: command not found", cmd->name);
      exit(EXIT_FAILURE);
    }
    execv(path, cmd->argv);
  } else {
    if (cmd->background) {
      g_child_pids[g_pid_count++] = pid;
    } else {
      waitpid(pid, NULL, 0);
    }
  }
}

static void execute_cmd(const command_t *cmd) {
  if (!cmd || !cmd->name)
    return;

  if (is_builtin(cmd)) {
    run_builtin(cmd);
  } else {
    run_external_cmd(cmd);
  }
}

void handle_input(const char *input) {

  // Reset global child pids
  g_pid_count = 0;
  for (int i = 0; i < MAX_CMDS; i++) {
    g_child_pids[i] = -1;
  }

  int num_tokens = 0;
  token_t *tokens = tokenize_input(input, &num_tokens);
  if (!tokens)
    return;

  // Step 2: Parse the command from tokens
  int num_cmds = 0;
  command_t **cmd_list = parse_commands(tokens, num_tokens, &num_cmds);
  if (!cmd_list) {
    fprintf(stderr, "Error parsing commands\n");
    free_tokens(tokens, num_tokens);
    return;
  }

  for (int i = 0; i < num_cmds; i++) {
    execute_cmd(cmd_list[i]);
  }

  for (int i = 0; i < g_pid_count; i++) {
    waitpid(g_child_pids[i], NULL, 0);
  }

  // Free tokens
  free_tokens(tokens, num_tokens);
  // Free command structure
  free_commands(cmd_list, num_cmds);
}
