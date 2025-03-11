#ifndef PARSER_H
#define PARSER_H

#include "types.h"

command_t *tokenize(const char *input);
int parse(command_t *cmd);

#endif /* PARSER_H */
