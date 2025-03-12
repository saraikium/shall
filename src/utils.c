#include "utils.h"
#include <string.h>

// Check if the file exists and is executable
int is_executable(const char *path) { return access(path, X_OK); }

char *find_path(const char *command) {
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

void fork_and_exec_cmd(command_t *cmd, char *full_path) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
  }

  if (pid == 0) {
    char *infile = strdup(cmd->infile);
    char *outfile = strdup(cmd->outfile);
    int append_out = cmd->append_out;

    if (infile) {
      int fd_in = open(cmd->infile, O_RDONLY);
      if (fd_in < 0) {
        perror("input file");
        exit(EXIT_FAILURE);
      }
      dup2(fd_in, STDIN_FILENO);
      close(fd_in);
    }
    if (outfile) {
      int flags = O_WRONLY | O_CREAT;
      flags |= append_out ? O_APPEND : O_TRUNC;

      int fd_out = open(cmd->outfile, flags);
      if (fd_out < 0) {
        perror("Error opening outfile:");
        exit(EXIT_FAILURE);
      }
      dup2(fd_out, STDOUT_FILENO);
      close(fd_out);
    }
    execv(full_path, cmd->argv);
    perror("exev");
    exit(EXIT_FAILURE);

  } else {
    int status;
    waitpid(pid, &status, 0);
  }
}
