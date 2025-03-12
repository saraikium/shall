#ifndef BUILTINS_H
#define BUILTINS_H

#include "types.h"

int handle_builtin(const command_t *cmd);
int is_builtin(const command_t *cmd);

#endif /* BUILTINS_H */
