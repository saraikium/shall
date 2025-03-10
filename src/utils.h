#ifndef UTILS_H
#define UTILS_H

#include "types.h"

int is_executable(const char *path);
char *find_path(const char *command);
void fork_and_exec_cmd(char *full_path, char **argv);

#endif /* UTILS_H */
