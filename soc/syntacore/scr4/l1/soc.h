/*
 * Copyright (C) 2024, Syntacore Ltd.
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
 * @brief SoC configuration macros for the Syntacore SCR4 core
 *
 * @author amr-sc
 */

#ifndef __SCR4_SOC_H_
#define __SCR4_SOC_H_

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */

#define PLF_MEM_BASE    (DT_REG_ADDR(DT_NODELABEL(ddr)))
#define PLF_MEM_SIZE    (DT_REG_SIZE(DT_NODELABEL(ddr)))
#define PLF_MEM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#define PLF_MEM_NAME    "DDR"

#define PLF_MMCFG_BASE  (DT_REG_ADDR(DT_NODELABEL(mmcfg)))
#define PLF_MMCFG_SIZE  (DT_REG_SIZE(DT_NODELABEL(mmcfg)))
#define PLF_MMCFG_ATTR (SCR_MPU_CTRL_MT_CFG | \
                        SCR_MPU_CTRL_MR |     \
                        SCR_MPU_CTRL_MW)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE  (DT_REG_ADDR(DT_NODELABEL(mmio)))
#define PLF_MMIO_SIZE  (DT_REG_SIZE(DT_NODELABEL(mmio)))
#define PLF_MMIO_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_MMIO_NAME   "MMIO"

#define PLF_OCRAM_BASE    (DT_REG_ADDR(DT_NODELABEL(ocram)))
#define PLF_OCRAM_SIZE    (DT_REG_SIZE(DT_NODELABEL(ocram)))
#define PLF_OCRAM_ATTR  (SCR_MPU_CTRL_MT_WEAKLY | \
                         SCR_MPU_CTRL_MA | \
                         SCR_MPU_CTRL_SR)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}

#endif /* __SCR4_SOC_H_ */
