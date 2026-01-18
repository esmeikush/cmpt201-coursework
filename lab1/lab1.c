#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

  // using getline()
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  printf("Please enter your line: ");
  read = getline(&line, &len, stdin);

  if (read != -1) {
    printf("You entered line %s ", line);

    // reference for strtok_r()

    char *str = line;
    char *token, *saveptr;

    for (int i = 1;; i++, str = NULL) {
      token = strtok_r(str, " ", &saveptr);
      if (token == NULL) {
        break;
      }
      printf("Token %d: %s\n", i, token);
    }
  } else
    printf("Failed to read line with tokenization\n");

  free(line);
  return 0;
}
