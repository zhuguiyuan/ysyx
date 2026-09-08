#include "state.h"
#include "dbg.h"
#include "inst.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void print_state(state_t *s) {
  uint32_t binary = s->mem[s->pc >> 2];
  inst_t inst = decode_from_binary(binary);
  g_autofree char *str = inst_asprint(&inst);

  printf("> PC = 0x%08x, INST = 0x%08x, STR = %s\n", s->pc, binary, str);
  for (int i = 0; i < 32; i += 4) {
    for (int j = 0; j < 4; ++j) {
      printf("%4s = %10u (0x%08x)%s", reg_name(i + j), s->registers[i + j],
             s->registers[i + j], j == 3 ? "\n" : ", ");
    }
  }
}

bool inst_cycle(state_t *s) {
  check(s->pc + 4 <= s->mem_size, "pc 0x%08x out of memory", s->pc);

  uint32_t binary = s->mem[s->pc >> 2];
  inst_t inst = decode_from_binary(binary);

  uint32_t *regs = s->registers;
  uint32_t pc_next = s->pc + 4;
  switch (inst.kind) {
  case INST_ADD:
    regs[inst.r.rd] = regs[inst.r.rs1] + regs[inst.r.rs2];
    break;
  case INST_ADDI:
    regs[inst.i.rd] = regs[inst.i.rs1] + inst.i.imm;
    break;
  case INST_LUI:
    regs[inst.u.rd] = inst.u.imm;
    break;
  case INST_LW: {
    uint32_t addr = regs[inst.i.rs1] + inst.i.imm;
    check((addr & 0b11) == 0, "misaligned lw address: 0x%08x", addr);
    check(addr + 4 <= s->mem_size, "lw address out of memory: 0x%08x", addr);
    regs[inst.i.rd] = s->mem[addr >> 2];
    break;
  }
  case INST_LBU: {
    uint32_t addr = regs[inst.i.rs1] + inst.i.imm;
    check(addr < s->mem_size, "lbu address out of memory: 0x%08x", addr);
    regs[inst.i.rd] = ((uint8_t *)s->mem)[addr];
    break;
  }
  case INST_SW: {
    uint32_t addr = regs[inst.s.rs1] + inst.s.imm;
    check((addr & 0b11) == 0, "misaligned sw address: 0x%08x", addr);
    check(addr + 4 <= s->mem_size, "sw address out of memory: 0x%08x", addr);
    s->mem[addr >> 2] = regs[inst.s.rs2];
    break;
  }
  case INST_SB: {
    uint32_t addr = regs[inst.s.rs1] + inst.s.imm;
    check(addr < s->mem_size, "sb address out of memory: 0x%08x", addr);
    ((uint8_t *)s->mem)[addr] = regs[inst.s.rs2];
    break;
  }
  case INST_JALR: {
    uint32_t target = (regs[inst.i.rs1] + (uint32_t)inst.i.imm) & ~1u;
    check((target & 0b11) == 0, "misaligned jalr target: 0x%08x", target);
    regs[inst.i.rd] = s->pc + 4;
    pc_next = target;
    break;
  }
  case INST_EBREAK:
    return false;
  }
  regs[0] = 0; /* x0 恒为 0 */
  s->pc = pc_next;
  return true;
error:
  abort();
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(FILE, fclose)
bool load_program(state_t *s, const char *filename) {
  g_autoptr(FILE) file = fopen(filename, "rb");
  check(file != NULL, "failed to open %s", filename);

  uint8_t buf[4];
  size_t word = 0;
  while (fread(buf, 1, sizeof(buf), file) == sizeof(buf)) {
    check(word < s->mem_size / 4, "program too large for memory");
    /* 小端序打包成 32 位指令字 */
    s->mem[word++] = (uint32_t)buf[0] | (uint32_t)buf[1] << 8 |
                     (uint32_t)buf[2] << 16 | (uint32_t)buf[3] << 24;
  }
  check(feof(file), "failed to read %s", filename);

  s->pc = 0;
  return true;
error:
  return false;
}
