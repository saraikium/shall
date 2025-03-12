#ifndef UTILS_H
#define UTILS_H

#include "types.h"

int is_executable(const char *path);
char *find_path(const char *command);
void fork_and_exec_cmd(command_t *cmd, char *full_path);

#endif /* UTILS_H */
