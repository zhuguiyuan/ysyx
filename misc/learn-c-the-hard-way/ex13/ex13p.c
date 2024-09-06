#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("ERROR: You need one argument.\n");
    // this is how you abort a program
    return 1;
  }

  char *pc = argv[1];
  while (*pc != '\0') {
    char c = *pc;
    if ('A' <= c && c <= 'Z') {
      *pc = c - 'A' + 'a';
    }
    ++pc;
  }
  int i = 0;
  char letter;
  for (i = 0; letter = argv[1][i], letter != '\0'; i++) {
    switch (letter) {
    case 'a':
      printf("%d: 'A'\n", i);
      break;

    case 'e':
      printf("%d: 'E'\n", i);
      break;

    case 'i':
      printf("%d: 'I'\n", i);
      break;

    case 'o':
      printf("%d: 'O'\n", i);
      break;

    case 'u':
      printf("%d: 'U'\n", i);
      break;

    case 'y':
      if (i > 2) {
        // it's only sometimes Y
        printf("%d: 'Y'\n", i);
      }
      break;

    default:
      printf("%d: %c is not a vowel\n", i, letter);
    }
  }

  return 0;
}
