#include "utils.h"

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
