#include <stdio.h>
void f(int x, int y) { printf("x = %d, y = %d\n", x, y); }
int main() {
  int i = 1;
  f(i++, i++);
  return 0;
}
