/*
 * @file cache.h
 *
 * @brief Syntacore L1/L2/L3 cache interface
 *
 * @author amr-sc
 *
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCR_CACHE_H
#define SCR_CACHE_H

/******************************************************************************
 *                              Includes section
 */
#include <soc_common.h>
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */
/* cache control CSRs */
#define SCR_CSR_CACHE_GLBL 0xbd4
/* cache info CSRs */
#define SCR_CSR_CACHE_DSCR_L1 0xfc3

/* global cache's control bits */
#define CACHE_GLBL_L1I_EN (1 << 0)
#define CACHE_GLBL_L1D_EN (1 << 1)
#define CACHE_GLBL_L1I_INV (1 << 2)
#define CACHE_GLBL_L1D_INV (1 << 3)
#define CACHE_GLBL_ENABLE (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN)
#define CACHE_GLBL_DISABLE 0
#define CACHE_GLBL_INV (CACHE_GLBL_L1I_INV | CACHE_GLBL_L1D_INV)

/* L2$ registers */
#define L2_CSR_VER_IDX   0
#define L2_CSR_DESCR_IDX 1
#define L2_CSR_EN_IDX    4
#define L2_CSR_FLUSH_IDX 5
#define L2_CSR_INV_IDX   6
#define L2_CSR_BUSY_IDX  11
#define L2_CSR_SYSCO_IDX 40


#define L2_CSR_VER_OFFS   (L2_CSR_VER_IDX * 4)
#define L2_CSR_DESCR_OFFS (L2_CSR_DESCR_IDX * 4)
#define L2_CSR_EN_OFFS    (L2_CSR_EN_IDX * 4)
#define L2_CSR_FLUSH_OFFS (L2_CSR_FLUSH_IDX * 4)
#define L2_CSR_INV_OFFS   (L2_CSR_INV_IDX * 4)
#define L2_CSR_BUSY_OFFS  (L2_CSR_BUSY_IDX * 4)
#define L2_CSR_SYSCO_OFFS (L2_CSR_SYSCO_IDX * 4)

#define L2_CSR_VER   (PLF_L2CTL_BASE + L2_CSR_VER_OFFS)
#define L2_CSR_DESCR (PLF_L2CTL_BASE + L2_CSR_DESCR_OFFS)
#define L2_CSR_EN    (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_FLUSH (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_INV   (PLF_L2CTL_BASE + L2_CSR_INV_OFFS)

#define L2_CSR_DESCR_OFFS_BANKS      (16)
#define L2_CSR_DESCR_OFFS_WAYS       (0)
#define L2_CSR_DESCR_OFFS_LINESZ_LG2 (4)
#define L2_CSR_DESCR_OFFS_LINES_LG2  (8)
#define L2_CSR_DESCR_OFFS_CORES      (28)
#define L2_CSR_DESCR_OFFS_TYPE       (13)

#define L2_CSR_DESCR_MASK_BANKS      (0xf)
#define L2_CSR_DESCR_MASK_WAYS       (0x7)
#define L2_CSR_DESCR_MASK_LINESZ_LG2 (0xf)
#define L2_CSR_DESCR_MASK_LINES_LG2  (0x1f)
#define L2_CSR_DESCR_MASK_CORES      (0xf)
#define L2_CSR_DESCR_MASK_TYPE       (0x7)

#define SCR_L2_DESCR_BANKS(val)		(((val >> L2_CSR_DESCR_OFFS_BANKS) & \
						L2_CSR_DESCR_MASK_BANKS) + 1)
#define SCR_L2_DESCR_WAYS(val)		(1UL << ((val >> L2_CSR_DESCR_OFFS_WAYS) & \
						L2_CSR_DESCR_MASK_WAYS))
#define SCR_L2_DESCR_LINESZ(val)	(1UL << ((val >> L2_CSR_DESCR_OFFS_LINESZ_LG2) & \
						L2_CSR_DESCR_MASK_LINESZ_LG2))
#define SCR_L2_DESCR_LINES(val)		(1UL << ((val >> L2_CSR_DESCR_OFFS_LINES_LG2) & \
						L2_CSR_DESCR_MASK_LINES_LG2))
#define SCR_L2_DESCR_CORES(val)		(((val >> L2_CSR_DESCR_OFFS_CORES) & \
						L2_CSR_DESCR_MASK_CORES) + 1)

/* L3$ registers */
#if IS_ENABLED(CONFIG_SCR_L3_CACHE)
#define L3_REG_SIZE          (8)

#define L3C_VID_IDX          (0)
#define L3C_DESCR_CACHE_IDX  (1)
#define L3C_DESCR_BANK_IDX   (2)

#define L3C_CMD_CTRL_IDX     (8)
#define L3C_CMD_ADDR_IDX     (9)
#define L3C_CMD_DATA0_IDX    (10)
#define L3C_CMD_DATA1_IDX    (11)
#define L3C_CMD_DATA2_IDX    (12)
#define L3C_CMD_DATA3_IDX    (13)
#define L3C_CMD_DATA4_IDX    (14)
#define L3C_CMD_DATA5_IDX    (15)
#define L3C_CMD_DATA6_IDX    (16)
#define L3C_CMD_DATA7_IDX    (17)

#define L3C_VID_OFFS         (L3C_VID_IDX * L3_REG_SIZE)
#define L3C_DESCR_CACHE_OFFS (L3C_DESCR_CACHE_IDX * L3_REG_SIZE)
#define L3C_DESCR_BANK_OFFS  (L3C_DESCR_BANK_IDX * L3_REG_SIZE)

#define L3C_CMD_CTRL_OFFS    (L3C_CMD_CTRL_IDX * L3_REG_SIZE)
#define L3C_CMD_ADDR_OFFS    (L3C_CMD_ADDR_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA0_OFFS   (L3C_CMD_DATA0_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA1_OFFS   (L3C_CMD_DATA1_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA2_OFFS   (L3C_CMD_DATA2_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA3_OFFS   (L3C_CMD_DATA3_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA4_OFFS   (L3C_CMD_DATA4_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA5_OFFS   (L3C_CMD_DATA5_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA6_OFFS   (L3C_CMD_DATA6_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA7_OFFS   (L3C_CMD_DATA7_IDX * L3_REG_SIZE)

#define L3C_VID              (PLF_L3CTL_BASE + L3C_VID_OFFS)
#define L3C_DESCR_CACHE      (PLF_L3CTL_BASE + L3C_DESCR_CACHE_OFFS)
#define L3C_DESCR_BANK       (PLF_L3CTL_BASE + L3C_DESCR_BANK_OFFS)

#define L3C_CMD_CTRL         (PLF_L3CTL_BASE + L3C_CMD_CTRL_OFFS)
#define L3C_CMD_ADDR         (PLF_L3CTL_BASE + L3C_CMD_ADDR_OFFS)
#define L3C_CMD_DATA0        (PLF_L3CTL_BASE + L3C_CMD_DATA0_OFFS)
#define L3C_CMD_DATA1        (PLF_L3CTL_BASE + L3C_CMD_DATA1_OFFS)
#define L3C_CMD_DATA2        (PLF_L3CTL_BASE + L3C_CMD_DATA2_OFFS)
#define L3C_CMD_DATA3        (PLF_L3CTL_BASE + L3C_CMD_DATA3_OFFS)
#define L3C_CMD_DATA4        (PLF_L3CTL_BASE + L3C_CMD_DATA4_OFFS)
#define L3C_CMD_DATA5        (PLF_L3CTL_BASE + L3C_CMD_DATA5_OFFS)
#define L3C_CMD_DATA6        (PLF_L3CTL_BASE + L3C_CMD_DATA6_OFFS)
#define L3C_CMD_DATA7        (PLF_L3CTL_BASE + L3C_CMD_DATA7_OFFS)

#define L3C_DESCR_CACHE_OFFS_CPU_NUM    (0)
#define L3C_DESCR_CACHE_OFFS_BANK_NUM   (8)
#define L3C_DESCR_CACHE_OFFS_MEM_NUM    (16)
#define L3C_DESCR_CACHE_OFFS_IO_NUM     (24)

#define L3C_DESCR_CACHE_MASK_CPU_NUM    (0xff)
#define L3C_DESCR_CACHE_MASK_BANK_NUM   (0xff)
#define L3C_DESCR_CACHE_MASK_MEM_NUM    (0xff)
#define L3C_DESCR_CACHE_MASK_IO_NUM     (0xff)

#define L3C_DESCR_BANK_OFFS_IDX_NUM    (0)
#define L3C_DESCR_BANK_OFFS_WAY_NUM    (8)
#define L3C_DESCR_BANK_OFFS_LINE_WIDTH (16)
#define L3C_DESCR_BANK_OFFS_DAT_WIDTH  (24)
#define L3C_DESCR_BANK_OFFS_ADDR_WIDTH (32)

#define L3C_DESCR_BANK_MASK_IDX_NUM    (0xff)
#define L3C_DESCR_BANK_MASK_WAY_NUM    (0xff)
#define L3C_DESCR_BANK_MASK_LINE_WIDTH (0xff)
#define L3C_DESCR_BANK_MASK_DAT_WIDTH  (0xff)
#define L3C_DESCR_BANK_MASK_ADDR_WIDTH (0xff)

#define L3C_OPCODE_NOP                  (0x00)
#define L3C_OPCODE_FLUASH               (0x01)
#define L3C_OPCODE_INVALID              (0x02)
#define L3C_OPCODE_CLEAN                (0x03)
#define L3C_OPCODE_LOAD_ADDR            (0x04)
#define L3C_OPCODE_LOAD_DIRECT          (0x05)

#define L3C_CMD_CTRL_VECTOR_SHIFT       (8)
#define L3C_CMD_CTRL_ALL_BANKS          (0xFF)

#define L3C_CMD_INVALIDATE_ALL          ( (L3C_CMD_CTRL_ALL_BANKS << L3C_CMD_CTRL_VECTOR_SHIFT) | L3C_OPCODE_INVALID )

#define L3B_STEP                        (0x100)
#define L3B_PRIVATE_0_BANK              (PLF_L3CTL_BASE + L3B_STEP)
#define L3B_PRIVATE_N_BANK(N)           (L3B_PRIVATE_0_BANK + (N) * L3B_STEP)

#define L3B_STS_IDX                     (0)
#define L3B_STS_OFFS                    (L3B_STS_IDX * L3_REG_SIZE)
#define L3B_STS_N(N)                    (L3B_PRIVATE_N_BANK(N) + L3B_STS_OFFS)
#define L3B_MCP_STS_IDX                 (1)
#define L3B_MCP_STS_OFFS                (L3B_MCP_STS_IDX * L3_REG_SIZE)
#define L3B_MCP_STS_N(N)                (L3B_PRIVATE_N_BANK(N) + L3B_MCP_STS_OFFS)
#define L3B_CMD_STS_IDX                 (2)
#define L3B_CMD_STS_OFFS                (L3B_CMD_STS_IDX * L3_REG_SIZE)
#define L3B_CMD_STS_N(N)                (L3B_PRIVATE_N_BANK(N) + L3B_CMD_STS_OFFS)

#define L3B_CMD_STS_OFFS_DONE           (0)
#define L3B_CMD_STS_MASK_DONE           (0x1)
#define L3B_CMD_STS_OFFS_DENY           (4)
#define L3B_CMD_STS_MASK_DENY           (0x1)
#define L3B_CMD_STS_OFFS_MISS           (8)
#define L3B_CMD_STS_MASK_MISS           (0x1)

#endif /* IS_ENABLED(CONFIG_SCR_L3_CACHE) */

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Checks if L2 cache is enabled
 *
 * @return true - if L2 cache is enabled; false - otherwise
 */
bool scr_cache_l2_is_enabled(void);

/**
 * @brief Enables L2 cache
 */
void scr_cache_l2_enable(void);

/**
 * @brief Disables L2 cache
 */
void scr_cache_l2_disable(void);

/**
 * @brief L3 cache initialization
 */
void scr_cache_l3_init(void);

/******************************************************************************
 *                              Implementation section
 */

#if IS_ENABLED(CONFIG_SCR_L1_CACHE)
/**
 * @brief Checks if L1 cache is available
 *
 * @return true - if L1 cache is available; false - otherwise
 */
static ALWAYS_INLINE bool scr_cache_l1_available(void)
{
    return csr_read(SCR_CSR_CACHE_DSCR_L1) != 0;
}

/**
 * @brief Setup L1 cache global policy
 *
 * @param [in] ctrl_val - MEM_CTRL_GLOBAL register value
 */
static ALWAYS_INLINE void scr_cache_l1_ctrl(unsigned long ctrl_val)
{
    if (scr_cache_l1_available()) {
    	csr_write(SCR_CSR_CACHE_GLBL, ctrl_val);
        ifence();
    }
}

/**
 * @brief Checks if L1 cache is enabled
 *
 * @return true - if L1 cache is enabled; false - otherwise
 */
static ALWAYS_INLINE bool scr_cache_l1_enabled(void)
{
    if (!scr_cache_l1_available())
        return false;
    return csr_read(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN);
}

/**
 * @brief Enables L1 cache
 */
static ALWAYS_INLINE void scr_cache_l1_enable(void)
{
    scr_cache_l1_ctrl(CACHE_GLBL_ENABLE);
}

/**
 * @brief Disable L1 cache
 */
static ALWAYS_INLINE void scr_cache_l1_disable(void)
{
    scr_cache_l1_ctrl(CACHE_GLBL_DISABLE | CACHE_GLBL_INV);

    while (csr_read(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_INV))
    	;
}

#endif /* IS_ENABLED(CONFIG_SCR_L1_CACHE) */

#endif /* SCR_CACHE_H */
