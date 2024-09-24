/*
 * Copyright (C) 2023, Syntacore Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * @file emu_ldst.c
 *
 * @brief SW handling for misaligned LD/ST instructions
 *
 * @author mn-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/arch/riscv/csr.h>

/******************************************************************************
 *                              Defines section
 */
#define INSN_MATCH_LB           0x3
#define INSN_MASK_LB            0x707f
#define INSN_MATCH_LH           0x1003
#define INSN_MASK_LH            0x707f
#define INSN_MATCH_LW           0x2003
#define INSN_MASK_LW            0x707f
#define INSN_MATCH_LD           0x3003
#define INSN_MASK_LD            0x707f
#define INSN_MATCH_LBU          0x4003
#define INSN_MASK_LBU           0x707f
#define INSN_MATCH_LHU          0x5003
#define INSN_MASK_LHU           0x707f
#define INSN_MATCH_LWU          0x6003
#define INSN_MASK_LWU           0x707f
#define INSN_MATCH_SB           0x23
#define INSN_MASK_SB            0x707f
#define INSN_MATCH_SH           0x1023
#define INSN_MASK_SH            0x707f
#define INSN_MATCH_SW           0x2023
#define INSN_MASK_SW            0x707f
#define INSN_MATCH_SD           0x3023
#define INSN_MASK_SD            0x707f

#define INSN_MATCH_FLW          0x2007
#define INSN_MASK_FLW           0x707f
#define INSN_MATCH_FLD          0x3007
#define INSN_MASK_FLD           0x707f
#define INSN_MATCH_FLQ          0x4007
#define INSN_MASK_FLQ           0x707f
#define INSN_MATCH_FSW          0x2027
#define INSN_MASK_FSW           0x707f
#define INSN_MATCH_FSD          0x3027
#define INSN_MASK_FSD           0x707f
#define INSN_MATCH_FSQ          0x4027
#define INSN_MASK_FSQ           0x707f

#define INSN_MATCH_C_LD         0x6000
#define INSN_MASK_C_LD          0xe003
#define INSN_MATCH_C_SD         0xe000
#define INSN_MASK_C_SD          0xe003
#define INSN_MATCH_C_LW         0x4000
#define INSN_MASK_C_LW          0xe003
#define INSN_MATCH_C_SW         0xc000
#define INSN_MASK_C_SW          0xe003
#define INSN_MATCH_C_LDSP       0x6002
#define INSN_MASK_C_LDSP        0xe003
#define INSN_MATCH_C_SDSP       0xe002
#define INSN_MASK_C_SDSP        0xe003
#define INSN_MATCH_C_LWSP       0x4002
#define INSN_MASK_C_LWSP        0xe003
#define INSN_MATCH_C_SWSP       0xc002
#define INSN_MASK_C_SWSP        0xe003

#define INSN_MATCH_C_FLD        0x2000
#define INSN_MASK_C_FLD         0xe003
#define INSN_MATCH_C_FLW        0x6000
#define INSN_MASK_C_FLW         0xe003
#define INSN_MATCH_C_FSD        0xa000
#define INSN_MASK_C_FSD         0xe003
#define INSN_MATCH_C_FSW        0xe000
#define INSN_MASK_C_FSW         0xe003
#define INSN_MATCH_C_FLDSP      0x2002
#define INSN_MASK_C_FLDSP       0xe003
#define INSN_MATCH_C_FSDSP      0xa002
#define INSN_MASK_C_FSDSP       0xe003
#define INSN_MATCH_C_FLWSP      0x6002
#define INSN_MASK_C_FLWSP       0xe003
#define INSN_MATCH_C_FSWSP      0xe002
#define INSN_MASK_C_FSWSP       0xe003

#define SH_RD   7
#define SH_RS2  20
#define SH_RS2C 2

#define RVC_RS2S(instr) (8 + (((instr) >> SH_RS2C) & 7))
#define SHIFT_RIGHT(x, y) ((y) < 0 ? ((x) << -(y)) : ((x) >> (y)))
#define REG_NUM(instr, pos) (((instr) >> (pos)) & 0x1f)
#define REG_PTR(instr, pos, regs) ((unsigned long*)(regs) + REG_NUM(instr, pos))

#define GET_RS2(instr, regs) REG_PTR(instr, SH_RS2, regs)
#define GET_RS2S(instr, regs) REG_PTR(RVC_RS2S(instr), 0, regs)
#define GET_RS2C(instr, regs) REG_PTR(instr, SH_RS2C, regs)
#define SET_RD(instr, regs, val) (*REG_PTR(instr, SH_RD, regs) = (val))

#define SET_F32_RD(ins, val) set_f32_reg(ins, 7, val)
#define SET_F64_RD(ins, val) set_f64_reg(ins, 7, val)

#define GET_F32_RS2(ins, res) get_f32_reg(ins, 20, res)
#define GET_F32_RS2C(ins, res) get_f32_reg(ins, 2, res)
#define GET_F32_RS2S(ins, res) get_f32_reg(RVC_RS2S(ins), 0, res)
#define GET_F64_RS2(ins, res) get_f64_reg(ins, 20, res)
#define GET_F64_RS2C(ins, res) get_f64_reg(ins, 2, res)
#define GET_F64_RS2S(ins, res) get_f64_reg(RVC_RS2S(ins), 0, res)

/******************************************************************************
 *                              Types section
 */
struct reg_data {
    union {
        uint8_t bytes[8];
        unsigned long ulong;
        uint64_t u64;
    };
};

enum reg_type {
    rt_bmask = 3,
    rt_b16 = 1,
    rt_b32 = 2,
    rt_b64 = 3,
    rt_sign = (1 << 2),
    rt_fp = (1 << 3),
    rt_rvc = (1 << 4),

    rt_s16 = rt_b16 | rt_sign,
    rt_u16 = rt_b16,
    rt_s32 = rt_b32 | rt_sign,
    rt_u32 = rt_b32,
    rt_s64 = rt_b64 | rt_sign,
    rt_u64 = rt_b64,
    rt_f32 = rt_b32 | rt_fp,
    rt_f64 = rt_b64 | rt_fp,
};

/******************************************************************************
 *                              Implementation section
 */
#if __riscv_flen

static void get_f32_reg(unsigned long ins, long pos, unsigned long *res)
{
    uintptr_t tmp0, tmp1;
    unsigned long offset = SHIFT_RIGHT(ins, (pos) - 3) & 0xf8;

    __asm__ volatile(
        ".altmacro\n"
        ".macro get_f32 freg, reg\n"
        "fsw f\\freg, 0(\\reg)\n"
        ".endm\n"
        ".option push\n"
        ".option norelax\n"
        ".option norvc\n"
        ".subsection 4\n"
        ".align 3\n"
        "1: \n"
        "fregn = 0\n"
        ".rept 32\n"
        "get_f32 %%(fregn), %[res]\n"
        "jr %[tmp0]\n"
        "fregn = fregn+1\n"
        ".endr\n"
        ".previous\n"
        "2: auipc %[tmp1], %%pcrel_hi(1b)\n"
        "add %[tmp1], %[tmp1], %[offset]\n"
        "jalr %[tmp0], %[tmp1], %%pcrel_lo(2b)\n"
        ".option pop\n"
        : [tmp0] "=&r" (tmp0),
          [tmp1] "=&r" (tmp1)
        : [res] "r" (res), [offset] "r" (offset)
        : "memory"
        );
}

static void set_f32_reg(unsigned long ins, long pos, unsigned long *value)
{
    uintptr_t tmp0, tmp1;
    unsigned long offset = SHIFT_RIGHT(ins, (pos) - 3) & 0xf8;

    __asm__ volatile(
        ".altmacro\n"
        ".macro put_f32 freg, reg\n"
        "flw f\\freg, 0(\\reg)\n"
        ".endm\n"
        ".option push\n"
        ".option norelax\n"
        ".option norvc\n"
        ".subsection 4\n"
        ".align 3\n"
        "1: \n"
        "fregn = 0\n"
        ".rept 32\n"
        "put_f32 %%(fregn), %[value]\n"
        "jr %[tmp0]\n"
        "fregn = fregn+1\n"
        ".endr\n"
        ".previous\n"
        "2: auipc %[tmp1], %%pcrel_hi(1b)\n"
        "add %[tmp1], %[tmp1], %[offset]\n"
        "jalr %[tmp0], %[tmp1], %%pcrel_lo(2b)\n"
        ".option pop\n"
        : [tmp0] "=&r" (tmp0),
          [tmp1] "=&r" (tmp1)
        : [value] "r" (value), [offset] "r" (offset)
        : "memory"
        );
}

static void get_f64_reg(unsigned long ins, long pos, uint64_t *res)
{
    uintptr_t tmp0, tmp1;
    unsigned long offset = SHIFT_RIGHT(ins, (pos) - 3) & 0xf8;

    __asm__ volatile(
        ".altmacro\n"
        ".macro get_f64 freg, reg\n"
        "fsd f\\freg, 0(\\reg)\n"
        ".endm\n"
        ".option push\n"
        ".option norelax\n"
        ".option norvc\n"
        ".subsection 4\n"
        ".align 3\n"
        "1: \n"
        "fregn = 0\n"
        ".rept 32\n"
        "get_f64 %%(fregn), %[res]\n"
        "jr %[tmp0]\n"
        "fregn = fregn+1\n"
        ".endr\n"
        ".previous\n"
        "2: auipc %[tmp1], %%pcrel_hi(1b)\n"
        "add %[tmp1], %[tmp1], %[offset]\n"
        "jalr %[tmp0], %[tmp1], %%pcrel_lo(2b)\n"
        ".option pop\n"
        : [tmp0] "=&r" (tmp0),
          [tmp1] "=&r" (tmp1)
        : [res] "r" (res), [offset] "r" (offset)
        : "memory"
        );
}

static void set_f64_reg(unsigned long ins, long pos, uint64_t *value)
{
    uintptr_t tmp0, tmp1;
    unsigned long offset = SHIFT_RIGHT(ins, (pos) - 3) & 0xf8;

    __asm__ volatile(
        ".altmacro\n"
        ".macro put_f64 freg, reg\n"
        "fld f\\freg, 0(\\reg)\n"
        ".endm\n"
        ".option push\n"
        ".option norelax\n"
        ".option norvc\n"
        ".subsection 4\n"
        ".align 3\n"
        "1: \n"
        "fregn = 0\n"
        ".rept 32\n"
        "put_f64 %%(fregn), %[value]\n"
        "jr %[tmp0]\n"
        "fregn = fregn+1\n"
        ".endr\n"
        ".previous\n"
        "2: auipc %[tmp1], %%pcrel_hi(1b)\n"
        "add %[tmp1], %[tmp1], %[offset]\n"
        "jalr %[tmp0], %[tmp1], %%pcrel_lo(2b)\n"
        ".option pop\n"
        : [tmp0] "=&r" (tmp0),
          [tmp1] "=&r" (tmp1)
        : [value] "r" (value), [offset] "r" (offset)
        : "memory"
        );
}
#endif /* __riscv_flen */

static unsigned long __attribute__((noinline)) emu_get_instr(unsigned long addr)
{
    intptr_t tmp;
    intptr_t tmp_mtvec;
    intptr_t tmp_mstatus;
    unsigned long instr;

    __asm__ volatile(
        "1: auipc %[tmp_mtvec], %%pcrel_hi(2f)\n"
        "addi %[tmp_mtvec], %[tmp_mtvec], %%pcrel_lo(1b)\n"
        "csrrw %[tmp_mtvec], mtvec, %[tmp_mtvec]\n"
        "li %[tmp_mstatus], (1 << 19) | (1 << 17)\n" // set MPRV, MXR
        "csrrs %[tmp_mstatus], mstatus, %[tmp_mstatus]\n"
        "lhu %[instr], 0(%[addr])\n"
        "andi %[tmp], %[instr], 3\n"
        "addi %[tmp], %[tmp], -3\n"
        "bnez %[tmp], 3f\n"
        "lhu %[tmp], 2(%[addr])\n"
        "slli %[tmp], %[tmp], 16\n"
        "or %[instr], %[instr], %[tmp]\n"
        "j 3f\n"
        ".align 2\n"
        "2: li %[instr], 0\n" // temp trap handler
        "3: csrw mstatus, %[tmp_mstatus]\n"
        "csrw mtvec, %[tmp_mtvec]\n"
        : [instr] "=&r" (instr),
          [tmp] "=&r" (tmp),
          [tmp_mtvec] "=&r" (tmp_mtvec),
          [tmp_mstatus] "=&r" (tmp_mstatus)
        : [addr] "r" (addr)
        : "memory");

    return instr;
}

// read byte, return byte or <0 - exc, mcause updated
static long __attribute__((noinline)) emu_load8(unsigned long addr)
{
    long val;
    intptr_t tmp_mtvec;
    intptr_t tmp_mstatus;

    __asm__ volatile(
        "1: auipc %[tmp_mtvec], %%pcrel_hi(2f)\n"
        "addi %[tmp_mtvec], %[tmp_mtvec], %%pcrel_lo(1b)\n"
        "csrrw %[tmp_mtvec], mtvec, %[tmp_mtvec]\n"
        "li %[tmp_mstatus], (1 << 17)\n" // set MPRV
        "csrrs %[tmp_mstatus], mstatus, %[tmp_mstatus]\n"
        "lbu %[val], 0(%[addr])\n"
        "j 3f\n"
        ".align 2\n"
        "2: li %[val], -1\n" // temp trap handler
        "3: csrw mstatus, %[tmp_mstatus]\n"
        "csrw mtvec, %[tmp_mtvec]\n"
        : [val] "=&r" (val),
          [tmp_mtvec] "=&r" (tmp_mtvec),
          [tmp_mstatus] "=&r" (tmp_mstatus)
        : [addr] "r" (addr)
        : "memory");

    return val;
}

// read byte, return -1 - exc, mcause updated
static long __attribute__((noinline)) emu_store8(unsigned long addr, uint8_t v)
{
    intptr_t tmp_mtvec;
    intptr_t tmp_mstatus;
    long val = v;

    __asm__ volatile(
        "1: auipc %[tmp_mtvec], %%pcrel_hi(2f)\n"
        "addi %[tmp_mtvec], %[tmp_mtvec], %%pcrel_lo(1b)\n"
        "csrrw %[tmp_mtvec], mtvec, %[tmp_mtvec]\n"
        "li %[tmp_mstatus], (1 << 17)\n" // set MPRV
        "csrrs %[tmp_mstatus], mstatus, %[tmp_mstatus]\n"
        "sb %[val], 0(%[addr])\n"
        "j 3f\n"
        ".align 2\n"
        "2: li %[val], -1\n" // temp trap handler
        "3: csrw mstatus, %[tmp_mstatus]\n"
        "csrw mtvec, %[tmp_mtvec]\n"
        : [val] "+r" (val),
          [tmp_mtvec] "=&r" (tmp_mtvec),
          [tmp_mstatus] "=&r" (tmp_mstatus)
        : [addr] "r" (addr)
        : "memory");

    return val;
}

/**
 * @brief Misaligned LD handling
 *
 * @param [in] regs - pointer to x0-x31 registers values.
 *		The layout should conform RISC-V ISA
 *
 * @return 0 - on success; 1 - otherwise
 */
int riscv_misaligned_load_process(uintptr_t* regs)
{
    uintptr_t ins_addr = csr_read(mepc);
    uintptr_t mem_addr = csr_read(mtval);

    unsigned long ins = emu_get_instr(ins_addr);
    unsigned ins_len = (ins & 3) == 3 ? 4 : 2;
    unsigned long reg_val = 0;
    enum reg_type reg_type;
    unsigned mem_len;
#if __riscv_flen
    struct reg_data val;
#endif /* __riscv_flen */

    if (ins) {
        /* Decode instruction */

        if ((ins & INSN_MASK_LW) == INSN_MATCH_LW) {
            reg_type = rt_s32;
#if __riscv_xlen == 64
        } else if ((ins & INSN_MASK_LD) == INSN_MATCH_LD) {
            reg_type = rt_s64;
        } else if ((ins & INSN_MASK_LWU) == INSN_MATCH_LWU) {
            reg_type = rt_u32;
#endif /* __riscv_xlen == 64 */
#if __riscv_flen
        } else if ((ins & INSN_MASK_FLD) == INSN_MATCH_FLD) {
            reg_type = rt_f64;
        } else if ((ins & INSN_MASK_FLW) == INSN_MATCH_FLW) {
            reg_type = rt_f32;
#endif /* __riscv_flen */
        } else if ((ins & INSN_MASK_LH) == INSN_MATCH_LH) {
            reg_type = rt_s16;
        } else if ((ins & INSN_MASK_LHU) == INSN_MATCH_LHU) {
            reg_type = rt_u16;
#if __riscv_xlen >= 64
        } else if ((ins & INSN_MASK_C_LD) == INSN_MATCH_C_LD) {
            reg_type = rt_s64 | rt_rvc;
        } else if ((ins & INSN_MASK_C_LDSP) == INSN_MATCH_C_LDSP && REG_NUM(ins, SH_RD)) {
            reg_type = rt_s64;
#endif /* __riscv_xlen >= 64 */
        } else if ((ins & INSN_MASK_C_LW) == INSN_MATCH_C_LW) {
            reg_type = rt_s32 | rt_rvc;
        } else if ((ins & INSN_MASK_C_LWSP) == INSN_MATCH_C_LWSP && REG_NUM(ins, SH_RD)) {
            reg_type = rt_s32;
#if __riscv_flen
        } else if ((ins & INSN_MASK_C_FLD) == INSN_MATCH_C_FLD) {
            reg_type = rt_f64 | rt_rvc;
        } else if ((ins & INSN_MASK_C_FLDSP) == INSN_MATCH_C_FLDSP) {
            reg_type = rt_f64;
#if __riscv_xlen == 32
        } else if ((ins & INSN_MASK_C_FLW) == INSN_MATCH_C_FLW) {
            reg_type = rt_f32 | rt_rvc;
        } else if ((ins & INSN_MASK_C_FLWSP) == INSN_MATCH_C_FLWSP) {
            reg_type = rt_f32;
#endif /* __riscv_xlen == 32 */
#endif /* __riscv_flen */
        } else {
            /* unknown */
            return 1;
        }

        mem_len = 1 << (reg_type & rt_bmask);

#if __riscv_flen
        val.u64 = 0;
#endif /* __riscv_flen */
        for (unsigned i = 0; i < mem_len; ++i) {
            long v = emu_load8(mem_addr + i);
            if (v < 0) {
				return 1;
            }

            if ((reg_type & rt_fp) == 0) {
                reg_val |= ((unsigned long)v & 0xff) << (i * 8);
#if __riscv_flen
            } else {
                val.bytes[i] = (uint8_t)v;
#endif /* __riscv_flen */
            }
        }

        if ((reg_type & rt_fp) == 0) {
            if (reg_type & rt_sign) {
                unsigned shift = 8 * (sizeof(intptr_t) - mem_len);
                reg_val = (long)(reg_val << shift) >> shift;
            }
        }

    } else {
        return 1;
    }

    if (reg_type & rt_rvc) {
        ins = RVC_RS2S(ins) << SH_RD;
    }

    if ((reg_type & rt_fp) == 0) {
        SET_RD(ins, regs, reg_val);
#if __riscv_flen
    } else if ((reg_type & rt_bmask) == rt_b64) {
        SET_F64_RD(ins, &val.u64);
    } else {
        SET_F32_RD(ins, &val.ulong);
#endif /* __riscv_flen */
    }

    csr_write(mepc, ins_addr + ins_len);

    return 0;
}

/**
 * @brief Misaligned ST handling
 *
 * @param [in] regs - pointer to x0-x31 registers values.
 *		The layout should conform RISC-V ISA
 *
 * @return 0 - on success; 1 - otherwise
 */
int riscv_misaligned_store_process(uintptr_t* regs)
{
    uintptr_t ins_addr = csr_read(mepc);
    uintptr_t mem_addr = csr_read(mtval);

    unsigned long ins = emu_get_instr(ins_addr);
    unsigned ins_len = (ins & 3) == 3 ? 4 : 2;
    enum reg_type reg_type;
    struct reg_data val = {};
    unsigned long *reg_val = &val.ulong;

    if (ins) {
        /* Decode instruction */
        if ((ins & INSN_MASK_SW) == INSN_MATCH_SW) {
            reg_type = rt_s32;
            reg_val = GET_RS2(ins, regs);
#if __riscv_xlen == 64
        } else if ((ins & INSN_MASK_SD) == INSN_MATCH_SD) {
            reg_type = rt_s64;
            reg_val = GET_RS2(ins, regs);
#endif /* __riscv_xlen == 64 */
#if __riscv_flen
        } else if ((ins & INSN_MASK_FSD) == INSN_MATCH_FSD) {
            reg_type = rt_f64;
            GET_F64_RS2(ins, &val.u64);
        } else if ((ins & INSN_MASK_FSW) == INSN_MATCH_FSW) {
            reg_type = rt_f32;
            GET_F32_RS2(ins, &val.ulong);
#endif /* __riscv_flen */
        } else if ((ins & INSN_MASK_SH) == INSN_MATCH_SH) {
            reg_type = rt_s16;
            reg_val = GET_RS2(ins, regs);
#if __riscv_xlen >= 64
        } else if ((ins & INSN_MASK_C_SD) == INSN_MATCH_C_SD) {
            reg_type = rt_s64 | rt_rvc;
            reg_val = GET_RS2S(ins, regs);
        } else if ((ins & INSN_MASK_C_SDSP) == INSN_MATCH_C_SDSP) {
            reg_type = rt_s64 | rt_rvc;
            reg_val = GET_RS2C(ins, regs);
#endif /* __riscv_xlen >= 64 */
        } else if ((ins & INSN_MASK_C_SW) == INSN_MATCH_C_SW) {
            reg_type = rt_s32 | rt_rvc;
            reg_val = GET_RS2S(ins, regs);
        } else if ((ins & INSN_MASK_C_SWSP) == INSN_MATCH_C_SWSP) {
            reg_type = rt_s32 | rt_rvc;
            reg_val = GET_RS2C(ins, regs);
#if __riscv_flen
        } else if ((ins & INSN_MASK_C_FSD) == INSN_MATCH_C_FSD) {
            reg_type = rt_f64 | rt_rvc;
            GET_F64_RS2S(ins, &val.u64);
        } else if ((ins & INSN_MASK_C_FSDSP) == INSN_MATCH_C_FSDSP) {
            reg_type = rt_f64 | rt_rvc;
            GET_F64_RS2C(ins, &val.u64);
#if __riscv_xlen == 32
        } else if ((ins & INSN_MASK_C_FSW) == INSN_MATCH_C_FSW) {
            reg_type = rt_f32 | rt_rvc;
            GET_F32_RS2S(ins, &val.ulong);
        } else if ((ins & INSN_MASK_C_FSWSP) == INSN_MATCH_C_FSWSP) {
            reg_type = rt_f32 | rt_rvc;
            GET_F32_RS2C(ins, &val.ulong);
#endif /* __riscv_xlen == 32 */
#endif /* __riscv_flen */
        } else {
            return 1;
        }
    } else {
        return 1;
    }

    unsigned mem_len = 1 << (reg_type & rt_bmask);
    for (unsigned i = 0; i < mem_len; ++i) {
        long v = emu_store8(mem_addr + i, ((const uint8_t*)reg_val)[i]);
        if (v < 0) {
			return 1;
        }
    }

    csr_write(mepc, ins_addr + ins_len);

    return 0;
}
