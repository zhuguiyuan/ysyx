#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s <num>\n", argv[0]);
    return EXIT_FAILURE;
  }
  long long sum = 0;
  long long i = 1;
  long long til = atoll(argv[1]);
  do {
    sum = sum + i;
    i = i + 1;
  } while (i <= til);
  printf("sum(1, %lld) = %lld\n", til, sum);
  return 0;
}
