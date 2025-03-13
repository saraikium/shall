#include "parser.h"

void free_tokens(token_t *tokens, int count) {
  if (!tokens)
    return;
  for (int i = 0; i < count; i++) {
    free(tokens[i].value);
  }
  free(tokens);
}

// Tokenizer: Splits input into structured tokens
token_t *tokenize_input(const char *input, int *num_tokens) {
  token_t *tokens = malloc(MAX_TOKENS * sizeof(token_t));
  if (!tokens) {
    perror("malloc");
    return NULL;
  }

  const char *ptr = input;
  int token_count = 0, in_single_quotes = 0, in_double_quotes = 0;

  while (*ptr) {
    // Skip the spaces
    while (isspace(*ptr))
      ptr++;

    if (*ptr == '\0')
      break;

    char *buffer = malloc(MAX_ARG_LENGTH);
    if (!buffer) {
      perror("Memory allocation failed");
      return NULL;
    }

    char *buf_ptr = buffer;

    while (*ptr) {
      // Current character
      char c = *ptr;
      // Handle quotes
      if (c == '\'' && !in_double_quotes) {
        in_single_quotes = !in_single_quotes;
        ptr++;
        continue;
      }

      if (c == '"' && !in_single_quotes) {
        in_double_quotes = !in_double_quotes;
        ptr++;
        continue;
      }

      if (c == '\\' && !in_single_quotes && in_double_quotes) {
        ptr++;
        if (*ptr) {
          *buf_ptr++ = *ptr++;
        }
        continue;
      }

      // If we encounter a space outside quotes it's the end of the token
      if (isspace(c) && !in_single_quotes && !in_double_quotes) {
        break;
      }

      // Handle redirection tokens as >, < , | and &
      if (!in_single_quotes && !in_double_quotes) {

        if (c == '>' || c == '<' || c == '|' || c == '&') {
          if (buf_ptr != buffer)
            break;

          *buf_ptr++ = *ptr++;
          if (c == '>' && *ptr == '>')
            *buf_ptr++ = *ptr++; // Handle >>
          break;
        }
      }
      *buf_ptr++ = *ptr++;
    }
    *buf_ptr = '\0';

    // Detect unterminated quotes
    if (in_single_quotes || in_double_quotes) {
      fprintf(stderr, "Syntax error: Unterminated quote\n");
      free(buffer);
      free_tokens(tokens, token_count);
      return NULL;
    }

    // Assign token type
    token_type_t type = TOKEN_WORD;
    if (strcmp(buffer, "|") == 0)
      type = TOKEN_PIPE;
    else if (strcmp(buffer, ">") == 0)
      type = TOKEN_REDIR_OUT;
    else if (strcmp(buffer, ">>") == 0)
      type = TOKEN_REDIR_APPEND;
    else if (strcmp(buffer, "<") == 0)
      type = TOKEN_REDIR_IN;
    else if (strcmp(buffer, "&") == 0)
      type = TOKEN_BACKGROUND;

    tokens[token_count++] = (token_t){type, strdup(buffer)};
    free(buffer);

    if (token_count >= MAX_TOKENS)
      break;
  }
  tokens[token_count] = (token_t){TOKEN_EOF, NULL};
  *num_tokens = token_count;
  return tokens;
}

// Converts tokens into a command_t structure
command_t **parse_commands(token_t *tokens, int num_tokens, int *num_commands) {
  if (num_tokens == 0)
    return NULL;

  command_t **commands = malloc((num_tokens + 1) * sizeof(command_t *));
  if (!commands) {
    perror("malloc failed");
    return NULL;
  }

  int cmd_index = 0;
  command_t *cmd = NULL;
  int argc = 0;
  for (int i = 0; i < num_tokens; i++) {
    if (!cmd) {
      cmd = malloc(sizeof(command_t));
      if (cmd == NULL) {
        perror("Memory allocation failed");
        return NULL;
      }
      memset(cmd, 0, sizeof(command_t));
      cmd->argv = malloc((num_tokens + 1) * sizeof(char *));
      if (cmd->argv == NULL) {
        perror("Memory Allocation failed");
        return NULL;
      }
      cmd->argc = 0;
      cmd->background = 0;
    }

    if (tokens[i].type == TOKEN_BACKGROUND) {
      if (cmd) {
        cmd->argv[argc] = NULL;
        cmd->name = cmd->argv[0];
        cmd->argc = argc;
        cmd->background = 1;
        commands[cmd_index++] = cmd;
      }
      cmd = NULL;
      argc = 0;
      continue;
    }

    switch (tokens[i].type) {
    case TOKEN_WORD:
      cmd->argv[argc++] = strdup(tokens[i].value);
      break;
    case TOKEN_REDIR_IN:
      if (i + 1 < num_tokens && tokens[i + 1].type == TOKEN_WORD) {
        cmd->infile = strdup(tokens[++i].value);
      } else {
        fprintf(stderr, "Syntax Error: expected filename after '<'\n");
      }
      break;

    case TOKEN_REDIR_OUT:
      if (i + 1 < num_tokens && tokens[i + 1].type == TOKEN_WORD) {
        cmd->outfile = strdup(tokens[++i].value);
        cmd->append_out = 0;
      } else {
        fprintf(stderr, "Syntax Error: expected filename after '>'\n");
      }
      break;

    case TOKEN_REDIR_APPEND:
      if (i + 1 < num_tokens && tokens[i + 1].type == TOKEN_WORD) {
        cmd->outfile = strdup(tokens[++i].value);
        cmd->append_out = 1;
      } else {
        fprintf(stderr, "Syntax error: expected file after '>>'\n");
      }
      break;
    case TOKEN_EOF:
      if (cmd) {
        cmd->argv[argc] = NULL;
        cmd->argc = argc;
        cmd->name = cmd->argv[0];
        commands[cmd_index++] = cmd;
        cmd = NULL;
      }
      break;
    default:
      break;
    }
  }

  commands[cmd_index] = NULL;
  *num_commands = cmd_index;

  return commands;
}
