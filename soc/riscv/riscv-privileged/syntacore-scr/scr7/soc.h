/*
 * @file soc.h
 *
 * @brief SoC configuration macros for the Syntacore SCR7 core
 *
 * @author dz-sc
 *
 * Copyright (c) 2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __RISCV64_SCR7_SOC_H_
#define __RISCV64_SCR7_SOC_H_

/******************************************************************************
 *                              Includes section
 */
#include <soc_common.h>
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */
#define SCR_SYS_CLK_FREQ   CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC

#ifndef SCR_SYS_CLK_FREQ
# error SCR_SYS_CLK_FREQ  not defined
#endif

/**
 * memory configuration
 */

#if __riscv_xlen == 32
#define EXPAND32ADDR(x) (x)
#else
#define EXPAND32ADDR(x) (((((x) / 0x80000000) & 1) * 0xffffffff00000000) + (x))
#endif

#define PLF_MEM_BASE    EXPAND32ADDR(0)
#if __riscv_xlen == 32
#define PLF_MEM_SIZE    (2*1024UL*1024UL*1024UL)
#else
#define PLF_MEM_SIZE    (4*1024UL*1024UL*1024UL)
#endif
#define PLF_MEM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#define PLF_MEM_NAME    "DDR"

#define PLF_MMCFG_BASE  EXPAND32ADDR(0xf0040000)
#define PLF_MMCFG_SIZE (8*1024)
#define PLF_MMCFG_ATTR (SCR_MPU_CTRL_MT_CFG | \
                        SCR_MPU_CTRL_MR |     \
                        SCR_MPU_CTRL_MW)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE   EXPAND32ADDR(0xff000000)
#define PLF_MMIO_SIZE   (8*1024*1024)
#define PLF_MMIO_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_MMIO_NAME   "MMIO"

#define PLF_PLIC_BASE   EXPAND32ADDR(0xfe000000)
#define PLF_PLIC_SIZE   (16*1024*1024)
#define PLF_PLIC_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_PLIC_NAME   "PLIC"

#define PLF_OCRAM_BASE  EXPAND32ADDR(0xffff0000)
#define PLF_OCRAM_SIZE  (64*1024)
#define PLF_OCRAM_ATTR  (SCR_MPU_CTRL_MT_WEAKLY | \
                         SCR_MPU_CTRL_MA | \
                         SCR_MPU_CTRL_SR)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME},       \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}

#endif /* __RISCV64_SCR7_SOC_H_ */
