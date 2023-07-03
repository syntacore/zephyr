/*
 * @file mpu.h
 *
 * @brief Syntacore MPU interface
 *
 * @author mn-sc
 *
 * Copyright (c) 2015-2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCR_MPU_H
#define SCR_MPU_H

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/devicetree.h>
#include <zephyr/arch/riscv/csr.h>

/******************************************************************************
 *                              Defines section
 */

/* MPU CSRs */
#if DT_HAS_COMPAT_STATUS_OKAY(scr_mpu)
#define MPU_NODE DT_INST(0,scr_mpu)
#define SCR_CSR_MPU_BASE DT_REG_ADDR(MPU_NODE)
#define SCR_CSR_MPU_SEL  DT_REG_ADDR_BY_NAME(MPU_NODE, select)
#define SCR_CSR_MPU_CTRL DT_REG_ADDR_BY_NAME(MPU_NODE, control)
#define SCR_CSR_MPU_ADDR DT_REG_ADDR_BY_NAME(MPU_NODE, address)
#define SCR_CSR_MPU_MASK DT_REG_ADDR_BY_NAME(MPU_NODE, mask)
#define SCR_MPU_REGIONS  DT_PROP(MPU_NODE,num_mpu_regions)
#else
#if defined CONFIG_SCR_MPU
#error "No SCR MPU devicetree information"
#endif
#endif

/* MPU ctrl bits */
#define SCR_MPU_ACCESS_R (1 << 0)
#define SCR_MPU_ACCESS_W (1 << 1)
#define SCR_MPU_ACCESS_X (1 << 2)
/* MPU ctrl mode positions */
#define SCR_MPU_MODE_M 0
#define SCR_MPU_MODE_U 1
#define SCR_MPU_MODE_S 2

#define SCR_MPU_MODE_ACCESS(mode, access_bits) ((access_bits) << ((mode) * 3 + 1))

#define _SCR_MPU_CTRL_MR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_MW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_MX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_X)
#define _SCR_MPU_CTRL_UR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_UW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_UX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_X)
#define _SCR_MPU_CTRL_SR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_SW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_SX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_X)
/* memory access modes */
#define _SCR_MPU_CTRL_MT_WEAKLY   (0 << 16) /*  C, WO */
#define _SCR_MPU_CTRL_MT_STRONG   (1 << 16) /* NC, SO */
#define _SCR_MPU_CTRL_MT_COHERENT (2 << 16) /* NC, WO */
#define _SCR_MPU_CTRL_MT_CFG      (3 << 16)
/* MPU region lock bit */
#define _SCR_MPU_CTRL_LOCK        (1 << 31)
/* MPU region valid bit */
#define _SCR_MPU_CTRL_VALID       (1 << 0)
/* MPU complex access */
#define _SCR_MPU_CTRL_MA        (_SCR_MPU_CTRL_MR | _SCR_MPU_CTRL_MW | _SCR_MPU_CTRL_MX)
#define _SCR_MPU_CTRL_SA        (_SCR_MPU_CTRL_SR | _SCR_MPU_CTRL_SW | _SCR_MPU_CTRL_SX)
#define _SCR_MPU_CTRL_UA        (_SCR_MPU_CTRL_UR | _SCR_MPU_CTRL_UW | _SCR_MPU_CTRL_UX)
#define _SCR_MPU_CTRL_ALL       (_SCR_MPU_CTRL_MA | _SCR_MPU_CTRL_SA | _SCR_MPU_CTRL_UA)
#define _SCR_MPU_CTRL_AR        (_SCR_MPU_CTRL_MR | _SCR_MPU_CTRL_SR | _SCR_MPU_CTRL_UR)
#define _SCR_MPU_CTRL_AW        (_SCR_MPU_CTRL_MW | _SCR_MPU_CTRL_SW | _SCR_MPU_CTRL_UW)
#define _SCR_MPU_CTRL_AX        (_SCR_MPU_CTRL_MX | _SCR_MPU_CTRL_SX | _SCR_MPU_CTRL_UX)

/* MPU address/mask conversion macros (with sign extension) */
#define SCR_MPU_MK_ADDR32(addr) \
    ((((addr) / 4) | ((addr) & 0x80000000) | ((addr) & 0x80000000) / 2) & 0xffffffff)
#define SCR_MPU_MK_MASK32(size) ((~((size) - 1) / 4) & 0xffffffff)

#define SCR_MPU_MK_ADDR64(addr) ((addr) / 4)
#define SCR_MPU_MK_MASK64(size) (~((size) - 1) / 4)

#if __riscv_xlen == 32
#define SCR_MPU_MK_ADDR(addr) SCR_MPU_MK_ADDR32(addr)
#define SCR_MPU_MK_MASK(size) SCR_MPU_MK_MASK32(size)
#elif __riscv_xlen == 64
#define SCR_MPU_MK_ADDR(addr) SCR_MPU_MK_ADDR64(addr)
#define SCR_MPU_MK_MASK(size) SCR_MPU_MK_MASK64(size)
#else
#error MPU addr/mask conversion is not implemented
#endif

#define SCR_MPU_CTRL_VALID     _SCR_MPU_CTRL_VALID
#define SCR_MPU_CTRL_MR        _SCR_MPU_CTRL_MR
#define SCR_MPU_CTRL_MW        _SCR_MPU_CTRL_MW
#define SCR_MPU_CTRL_MX        _SCR_MPU_CTRL_MX
#define SCR_MPU_CTRL_UR        _SCR_MPU_CTRL_UR
#define SCR_MPU_CTRL_UW        _SCR_MPU_CTRL_UW
#define SCR_MPU_CTRL_UX        _SCR_MPU_CTRL_UX
#define SCR_MPU_CTRL_SR        _SCR_MPU_CTRL_SR
#define SCR_MPU_CTRL_SW        _SCR_MPU_CTRL_SW
#define SCR_MPU_CTRL_SX        _SCR_MPU_CTRL_SX
#define SCR_MPU_CTRL_MT_WEAKLY _SCR_MPU_CTRL_MT_WEAKLY
#define SCR_MPU_CTRL_MT_STRONG _SCR_MPU_CTRL_MT_STRONG
/* #define SCR_MPU_CTRL_MT_COHERENT _SCR_MPU_CTRL_MT_COHERENT */
#define SCR_MPU_CTRL_MT_CFG    _SCR_MPU_CTRL_MT_CFG
#define SCR_MPU_CTRL_LOCK      _SCR_MPU_CTRL_LOCK

#define SCR_MPU_CTRL_MA  _SCR_MPU_CTRL_MA
#define SCR_MPU_CTRL_SA  _SCR_MPU_CTRL_SA
#define SCR_MPU_CTRL_UA  _SCR_MPU_CTRL_UA
#define SCR_MPU_CTRL_ALL _SCR_MPU_CTRL_ALL
#define SCR_MPU_CTRL_AR  _SCR_MPU_CTRL_AR
#define SCR_MPU_CTRL_AW  _SCR_MPU_CTRL_AW
#define SCR_MPU_CTRL_AX  _SCR_MPU_CTRL_AX

/******************************************************************************
 *                              Types section
 */
/* platform memory configuration */
typedef struct scr_mem_region_info_struct {
    uintptr_t      base;
    uintptr_t      size;
    unsigned long  attr;
    const char    *name;
    unsigned long  mrt_attr;
} scr_mem_region_info;

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Active MPU memory region update.
 *		Region will not be unexpectedly disabled during the process
 *
 * @param [in] regn - region number
 * @param [in] base - region base address
 * @param [in] size - region size
 * @param [in] attr - region attributes
 */
static ALWAYS_INLINE void scr_mpu_region_update(long regn, uintptr_t base, unsigned long size, unsigned long attr)
{
	csr_write(SCR_CSR_MPU_SEL, regn);
	csr_write(SCR_CSR_MPU_CTRL, attr);
	csr_write(SCR_CSR_MPU_ADDR, SCR_MPU_MK_ADDR(base));
	csr_write(SCR_CSR_MPU_MASK, SCR_MPU_MK_MASK(size));
}

/**
 * @brief MPU memory region configuration
 *
 * @param [in] regn - region number
 * @param [in] base - region base address
 * @param [in] size - region size
 * @param [in] attr - region attributes
 */
static ALWAYS_INLINE void scr_mpu_region_setup(long regn, uintptr_t base, unsigned long size, unsigned long attr)
{
	csr_write(SCR_CSR_MPU_SEL, regn);
	csr_write(SCR_CSR_MPU_CTRL, 0);
	csr_write(SCR_CSR_MPU_ADDR, SCR_MPU_MK_ADDR(base));
	csr_write(SCR_CSR_MPU_MASK, SCR_MPU_MK_MASK(size));
	csr_write(SCR_CSR_MPU_CTRL, attr);
}

/**
 * @brief Early MPU initialization. Brings MPU to known state after possible
 *		warm reset
 */
static ALWAYS_INLINE void scr_mpu_early_init(void)
{
	/* region0= NC,SO, global */
	scr_mpu_region_update(0, 0, 0, SCR_MPU_CTRL_MA | SCR_MPU_CTRL_MT_STRONG | SCR_MPU_CTRL_VALID);
	ifence();
	/* region1= MMCFG */
	scr_mpu_region_setup(1, PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR | SCR_MPU_CTRL_VALID);
	/* region2= MMIO */
	scr_mpu_region_setup(2, PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR | SCR_MPU_CTRL_VALID);
	/* all other regions= disabled */
	for (unsigned rgn = 3; ; ++rgn) {
		csr_write(SCR_CSR_MPU_SEL, rgn);
		if (csr_read(SCR_CSR_MPU_SEL) == 0)
			break;
		csr_write(SCR_CSR_MPU_CTRL, 0);
	}
}

/**
 * @brief MPU configuration
 *
 * @param mem_regions[in] - array of memory region descriptors
 */
void scr_mpu_setup(const scr_mem_region_info *mem_regions);

#endif // SCR_MPU_H
