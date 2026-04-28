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
 * @brief SoC configuration macros for the Syntacore SCR6 core
 *
 * @author amr-sc
 */

#ifndef __SCR6_SOC_H_
#define __SCR6_SOC_H_

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */

#define PLF_MEM_BASE    (DT_REG_ADDR_U64(DT_NODELABEL(ddr)))
#define PLF_MEM_SIZE    (DT_REG_SIZE(DT_NODELABEL(ddr)))
#define PLF_MEM_ATTR    (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_MEM_NAME    "DDR"

#define PLF_MMCFG_BASE  (DT_REG_ADDR_U64(DT_NODELABEL(mmcfg)))
#define PLF_MMCFG_SIZE  (DT_REG_SIZE(DT_NODELABEL(mmcfg)))
#define PLF_MMCFG_ATTR  (PMP_R | PMP_W | SCR_PMP_CTRL_MT_CONFIG)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE  (DT_REG_ADDR_U64(DT_NODELABEL(mmio)))
#define PLF_MMIO_SIZE  (DT_REG_SIZE(DT_NODELABEL(mmio)))
#define PLF_MMIO_ATTR  (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_MMIO_NAME  "MMIO"

#define PLF_PLIC_BASE (DT_REG_ADDR_U64(DT_NODELABEL(plic)))
#define PLF_PLIC_SIZE (16*1024*1024)
#define PLF_PLIC_ATTR (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_PLIC_NAME "PLIC"

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME},       \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME}

#endif /* __SCR6_SOC_H_ */
