#ifndef PARSER_H
#define PARSER_H

#define MAX_ARG_LENGTH 1024
#define MAX_TOKENS 64

#include "types.h"

token_t *tokenize_input(const char *input, int *num_tokens);
command_t *parse_command(token_t *tokens, int num_tokens);
void free_tokens(token_t *tokens, int token_count);

#endif /* PARSER_H */
