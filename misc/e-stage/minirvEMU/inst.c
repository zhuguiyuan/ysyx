#include "inst.h"
#include "dbg.h"
#include <glib.h>
#include <stdint.h>
#include <stdlib.h>

static const char *const reg_name_dict[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

const char *reg_name(uint32_t reg_idx) {
  check(reg_idx < 32, "invlaid reg_id");
  return reg_name_dict[reg_idx];
error:
  return NULL;
}

static const char *imm_str(int32_t imm) {
  static char buf[16];
  g_snprintf(buf, sizeof(buf), imm < 0 ? "-0x%x" : "0x%x",
             imm < 0 ? ~(uint32_t)imm + 1 : (uint32_t)imm);
  return buf;
}

char *inst_asprint(const inst_t *inst) {
  switch (inst->kind) {
  case INST_ADD:
    return g_strdup_printf("add\t%s, %s, %s", reg_name(inst->rd),
                           reg_name(inst->rs1), reg_name(inst->rs2));
  case INST_ADDI:
    return g_strdup_printf("addi\t%s, %s, %s", reg_name(inst->rd),
                           reg_name(inst->rs1), imm_str(inst->imm));
  case INST_LUI:
    /* u.imm 存的是左移 12 位并符号扩展后的值, 打印时右移 12 位还原 */
    return g_strdup_printf("lui\t%s, %s", reg_name(inst->rd),
                           imm_str((uint32_t)inst->imm >> 12));
  case INST_LW:
    return g_strdup_printf("lw\t%s, %s(%s)", reg_name(inst->rd),
                           imm_str(inst->imm), reg_name(inst->rs1));
  case INST_LBU:
    return g_strdup_printf("lbu\t%s, %s(%s)", reg_name(inst->rd),
                           imm_str(inst->imm), reg_name(inst->rs1));
  case INST_SW:
    return g_strdup_printf("sw\t%s, %s(%s)", reg_name(inst->rs2),
                           imm_str(inst->imm), reg_name(inst->rs1));
  case INST_SB:
    return g_strdup_printf("sb\t%s, %s(%s)", reg_name(inst->rs2),
                           imm_str(inst->imm), reg_name(inst->rs1));
  case INST_JALR:
    return g_strdup_printf("jalr\t%s, %s(%s)", reg_name(inst->rd),
                           imm_str(inst->imm), reg_name(inst->rs1));
  case INST_EBREAK:
    return g_strdup_printf("ebreak");
  }
  sentinel("unreachable");
error:
  return NULL;
}

inst_t inst_decode_from_binary(uint32_t binary) {
  inst_t inst = {0};
  uint32_t opcode = binary & 0b1111111;
  uint32_t rd = (binary >> 7) & 0b11111;
  uint32_t funct3 = (binary >> 12) & 0b111;
  uint32_t rs1 = (binary >> 15) & 0b11111;
  uint32_t rs2 = (binary >> 20) & 0b11111;
  int32_t imm_i = (int32_t)binary >> 20;
  int32_t imm_s =
      ((int32_t)binary >> 25 << 5) | ((int32_t)(binary >> 7) & 0b11111);
  int32_t imm_u = (int32_t)(binary & 0xfffff000);

  switch (opcode) {
  case 0b0110011: /* OP: add */
    check(funct3 == 0b000 && (binary >> 25) == 0b0000000,
          "unsupported instruction: 0x%08x", binary);
    inst.kind = INST_ADD;
    inst.rd = rd;
    inst.rs1 = rs1;
    inst.rs2 = rs2;
    break;
  case 0b0010011: /* OP-IMM: addi */
    check(funct3 == 0b000, "unsupported instruction: 0x%08x", binary);
    inst.kind = INST_ADDI;
    inst.rd = rd;
    inst.rs1 = rs1;
    inst.imm = imm_i;
    break;
  case 0b0110111: /* LUI */
    inst.kind = INST_LUI;
    inst.rd = rd;
    inst.imm = imm_u;
    break;
  case 0b0000011: /* LOAD: lw / lbu */
    check(funct3 == 0b010 || funct3 == 0b100, "unsupported instruction: 0x%08x",
          binary);
    inst.kind = funct3 == 0b010 ? INST_LW : INST_LBU;
    inst.rd = rd;
    inst.rs1 = rs1;
    inst.imm = imm_i;
    break;
  case 0b0100011: /* STORE: sb / sw */
    check(funct3 == 0b000 || funct3 == 0b010, "unsupported instruction: 0x%08x",
          binary);
    inst.kind = funct3 == 0b000 ? INST_SB : INST_SW;
    inst.rs1 = rs1;
    inst.rs2 = rs2;
    inst.imm = imm_s;
    break;
  case 0b1100111: /* JALR */
    check(funct3 == 0b000, "unsupported instruction: 0x%08x", binary);
    inst.kind = INST_JALR;
    inst.rd = rd;
    inst.rs1 = rs1;
    inst.imm = imm_i;
    break;
  case 0b1110011: /* SYSTEM: 只支持 ebreak */
    check(binary == 0b00000000000100000000000001110011,
          "unsupported instruction: 0x%08x", binary);
    inst.kind = INST_EBREAK;
    inst.rd = rd;
    inst.rs1 = rs1;
    inst.imm = imm_i; /* ebreak 编码中为 1 */
    break;
  default:
    sentinel("unsupported opcode: 0x%08x", binary);
  }
  return inst;
error:
  exit(EXIT_FAILURE);
}
