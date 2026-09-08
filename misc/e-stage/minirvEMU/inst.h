#ifndef __inst_h__
#define __inst_h__

#include <stdint.h>

const char *reg_name(uint32_t reg_idx);

typedef enum {
  INST_ADD,
  INST_ADDI,
  INST_LUI,
  INST_LW,
  INST_LBU,
  INST_SW,
  INST_SB,
  INST_JALR,
  INST_EBREAK
} inst_kind_t;

typedef struct {
  inst_kind_t kind;
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  int32_t imm;
} inst_t;

char *inst_asprint(const inst_t *inst);
inst_t inst_decode_from_binary(uint32_t binary);

#endif
