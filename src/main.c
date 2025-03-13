#include "shall.h"

int main(void) {
  while (1) {
    printf("$ ");
    // Flush after every printf
    fflush(stdout);
    // Wait for user input
    char input[4096];
    clean_input(input, 4096);

    handle_input(input);
  }
  return 0;
}
