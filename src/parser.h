#ifndef PARSER_H
#define PARSER_H

#define MAX_ARG_LENGTH 4096
#define MAX_TOKENS 4096

#include "types.h"

token_t *tokenize_input(const char *input, int *num_tokens);
command_t **parse_commands(token_t *tokens, int num_tokens, int *num_commands);
void free_tokens(token_t *tokens, int token_count);

#endif /* PARSER_H */
