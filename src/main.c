#include "shall.h"

int main(int argc, char *argv[]) {
  while (1) {
    printf("$ ");
    // Flush after every printf
    fflush(stdout);
    // Wait for user input
    char input[100];
    clean_input(input, 100);

    handle_input(input);
  }
  return 0;
}
