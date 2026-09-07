#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t PC = 0;
uint8_t R[4];
uint8_t M[256];

void print_state(void) {
  printf("> PC = %d, INST = 0x%08b\n", PC, M[PC]);
  for (int i = 0; i < 4; ++i) {
    printf("R[%d] = %3d (0x%02x)\n", i, R[i], R[i]);
  }
}

/** @return (bool) program continue? */
bool inst_cycle(void) {
  uint8_t inst = M[PC];
  uint8_t rd = (inst >> 4) & 0b11;
  uint8_t rs1 = (inst >> 2) & 0b11;
  uint8_t rs2 = (inst >> 0) & 0b11;
  uint8_t imm = (inst >> 0) & 0b11;
  uint8_t s = (inst >> 2) & 0b11;
  uint8_t offset = (inst >> 2) & 0b1111;
  uint8_t sign_ext_offset = (int8_t)(offset << 4) >> 4;

  uint8_t new_pc = PC + 1;

  switch (inst >> 6) {
  case 0b00:
    R[rd] = R[rs1] + R[rs2];
    break;
  case 0b01:
    break;
  case 0b10:
    R[rd] = imm << (s << 1);
    break;
  case 0b11:
    if (R[0] != R[rs2]) {
      new_pc = PC + sign_ext_offset;
    }
    break;
  }
  if (PC == new_pc) {
    puts("halt detected");
    return false;
  }
  PC = new_pc;
  return true;
}

void init_program(void) {
  PC = 0;
  uint8_t prog[256] = {
      // assume r0 is ready
      //   0b10000110, // li r0, 8
      //   0b10010010, // li r1, 2
      //   0b00000001, // add r0, r0, r1
      0b10010000, // li r1, 0
      0b10100000, // li r2, 0
      0b10110001, // li r3, 1
      0b00010111, // add r1, r1, r3
      0b00101001, // add r2, r2, r1
      0b11111001, // bner0 r1, -2
      0b01101000, // io led
      0b01101001, // io seg
      0b10000000, // li r0, 0
      0b11000011  // bner0 r3, 0
  };
  for (int i = 0; i < 16; ++i) {
    M[i] = prog[i];
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s <num>\n", argv[0]);
    return EXIT_FAILURE;
  }
  init_program();
  R[0] = atoi(argv[1]);
  while (inst_cycle()) {
  }
  print_state();
  return EXIT_SUCCESS;
}