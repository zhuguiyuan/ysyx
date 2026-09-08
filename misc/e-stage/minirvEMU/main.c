#include "dbg.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  g_autoptr(state_t) s = NULL;
  check(argc == 2, "usage: %s <program.bin>", argv[0]);

  s = g_new0(state_t, 1);
  state_init(s, 1 << 20);
  check(load_program(s, argv[1]), "failed to load program %s", argv[1]);

  int max_cycle = 6000;
  int cycle_cnt = 0;
  while (++cycle_cnt < max_cycle && inst_cycle(s)) {
  }
  print_state(s);
  printf("cycle_cnt = %d\n", cycle_cnt);
  return EXIT_SUCCESS;
error:
  return EXIT_FAILURE;
}
