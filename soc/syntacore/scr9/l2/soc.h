/*
 * Copyright (C) 2022, Syntacore Ltd.
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
 * @file soc.h
 *
 * @brief SoC configuration macros for the Syntacore SCR9 core
 *
 * @author amr-sc
 */

#ifndef __RISCV64_SCR9_SOC_H_
#define __RISCV64_SCR9_SOC_H_

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */
#define SCR_SYS_CLK_FREQ   CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC

#define PLF_MEM_BASE    (UINT64_C(DT_REG_ADDR(DT_NODELABEL(ddr))))
#define PLF_MEM_SIZE    (UINT32_C(DT_REG_SIZE(DT_NODELABEL(ddr))))
#if IS_ENABLED(CONFIG_SCR_MPU)
#define PLF_MEM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#else
#define PLF_MEM_ATTR    (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#endif /* IS_ENABLED(CONFIG_SCR_MPU) */
#define PLF_MEM_NAME    "DDR"

#define PLF_MMCFG_BASE  (UINT64_C(DT_REG_ADDR(DT_NODELABEL(mmcfg))))
#define PLF_MMCFG_SIZE  (UINT32_C(DT_REG_SIZE(DT_NODELABEL(mmcfg))))
#if IS_ENABLED(CONFIG_SCR_MPU)
#define PLF_MMCFG_ATTR (SCR_MPU_CTRL_MT_CFG | \
                        SCR_MPU_CTRL_MR |     \
                        SCR_MPU_CTRL_MW)
#else
#define PLF_MMCFG_ATTR  (PMP_R | PMP_W | SCR_PMP_CTRL_MT_CONFIG)
#endif /* IS_ENABLED(CONFIG_SCR_MPU) */
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE  (UINT64_C(DT_REG_ADDR(DT_NODELABEL(mmio))))
#define PLF_MMIO_SIZE  (UINT32_C(DT_REG_SIZE(DT_NODELABEL(mmio))))
#if IS_ENABLED(CONFIG_SCR_MPU)
#define PLF_MMIO_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#else
#define PLF_MMIO_ATTR    (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#endif /* IS_ENABLED(CONFIG_SCR_MPU) */
#define PLF_MMIO_NAME   "MMIO"

#define PLF_PLIC_BASE (UINT64_C(DT_REG_ADDR(DT_NODELABEL(plic))))
#define PLF_PLIC_SIZE   (16*1024*1024)
#if IS_ENABLED(CONFIG_SCR_MPU)
#define PLF_PLIC_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#else
#define PLF_PLIC_ATTR   (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#endif /* IS_ENABLED(CONFIG_SCR_MPU) */
#define PLF_PLIC_NAME   "PLIC"

#define PLF_OCRAM_BASE    (UINT64_C(DT_REG_ADDR(DT_NODELABEL(ocram))))
#define PLF_OCRAM_SIZE    (UINT32_C(DT_REG_SIZE(DT_NODELABEL(ocram))))
#if IS_ENABLED(CONFIG_SCR_MPU)
#define PLF_OCRAM_ATTR  (SCR_MPU_CTRL_MT_WEAKLY | \
                         SCR_MPU_CTRL_MA | \
                         SCR_MPU_CTRL_SR)
#else
#define PLF_OCRAM_ATTR    (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#endif /* IS_ENABLED(CONFIG_SCR_MPU) */
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME},       \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}

#endif /* __RISCV64_SCR9_SOC_H_ */
