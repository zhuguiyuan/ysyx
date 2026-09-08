#ifndef __state_h_
#define __state_h_

#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  uint32_t registers[32];
  uint32_t pc;
  uint32_t mem_size;
  uint32_t *mem;
} state_t;

static inline void state_init(state_t *s, uint32_t mem_size) {
  memset(s, 0, sizeof(*s)); /* 清零寄存器和 pc */
  s->mem_size = mem_size;
  s->mem = g_malloc0(mem_size);
}

static inline void state_cleanup(state_t *s) {
  if (s != NULL) {
    g_free(s->mem);
    s->mem = NULL;
  }
}
G_DEFINE_AUTOPTR_CLEANUP_FUNC(state_t, state_cleanup)

void print_state(state_t *s);

/** @return (bool) program continue?
  When any error occurs (like access invalid address), exit with error. */
bool inst_cycle(state_t *s);

/** @return (bool) load program success?
  Program is a flat binary file and program's size must <= mem_size */
bool load_program(state_t *s, const char *filename);

#endif