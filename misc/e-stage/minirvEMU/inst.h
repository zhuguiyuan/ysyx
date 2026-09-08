#ifndef __inst_h__
#define __inst_h__

#include <stdint.h>

typedef enum { INST_FMT_R, INST_FMT_I, INST_FMT_S, INST_FMT_U } inst_fmt_t;

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
  union {
    struct {
      uint8_t rd;
      uint8_t rs1;
      uint8_t rs2;
    } r;
    struct {
      uint8_t rd;
      uint8_t rs1;
      int32_t imm;
    } i;
    struct {
      uint8_t rs1;
      uint8_t rs2;
      int32_t imm;
    } s;
    struct {
      uint8_t rd;
      int32_t imm;
    } u;
  };
} inst_t;

const char *reg_name(uint32_t reg_id);
inst_fmt_t inst_fmt_of(inst_kind_t k);
char *inst_asprint(const inst_t *inst);
inst_t decode_from_binary(uint32_t binary);

#endif
