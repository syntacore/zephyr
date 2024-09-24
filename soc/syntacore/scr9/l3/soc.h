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
 * @file soc.h
 *
 * @brief SoC configuration macros for the SCR9 L3 cluster
 *
 * @author amr-sc
 */

#ifndef __RISCV64_SCR9_L3_SOC_H_
#define __RISCV64_SCR9_L3_SOC_H_

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/sys/util.h>

/******************************************************************************
 *                              Defines section
 */
#define SCR_SYS_CLK_FREQ   CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC

/**
 * memory configuration
 */
#define PLF_MEM_BASE    (UINT64_C(DT_REG_ADDR(DT_NODELABEL(ddr))))
#define PLF_MEM_SIZE    (UINT32_C(DT_REG_SIZE(DT_NODELABEL(ddr))))
#define PLF_MEM_ATTR    (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_MEM_NAME    "DDR"

#define PLF_DDR2_BASE  (UINT64_C(DT_REG_ADDR(DT_NODELABEL(ddr2))))
#define PLF_DDR2_SIZE  (UINT32_C(DT_REG_SIZE(DT_NODELABEL(ddr2))))
#define PLF_DDR2_ATTR  (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_DDR2_NAME  "DDR2"

#define PLF_MMCFG_BASE  (UINT64_C(DT_REG_ADDR(DT_NODELABEL(mmcfg))))
#define PLF_MMCFG_SIZE  (UINT32_C(DT_REG_SIZE(DT_NODELABEL(mmcfg))))
#define PLF_MMCFG_ATTR  (PMP_R | PMP_W | SCR_PMP_CTRL_MT_CONFIG)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (UINT64_C(DT_REG_ADDR(DT_NODELABEL(mtimer))))
#define PLF_MTIMER_SIZE (UINT32_C(DT_REG_SIZE(DT_NODELABEL(mtimer))))
#define PLF_MTIMER_ATTR (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_MTIMER_NAME "MTIMER"

#define PLF_L2CTL_BASE   (PLF_MMCFG_BASE)

#define PLF_MMIO_BASE  (UINT64_C(DT_REG_ADDR(DT_NODELABEL(mmio))))
#define PLF_MMIO_SIZE  (UINT32_C(DT_REG_SIZE(DT_NODELABEL(mmio))))
#define PLF_MMIO_ATTR    (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_MMIO_NAME    "MMIO"

#define PLF_CLUSTER_CFG_BASE    (UINT64_C(DT_REG_ADDR(DT_NODELABEL(clcfg))))
#define PLF_CLUSTER_CFG_SIZE    (UINT32_C(DT_REG_SIZE(DT_NODELABEL(clcfg))))
#define PLF_CLUSTER_CFG_ATTR    (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)

#define PLF_CLUSTER_CFG_NAME "Cluster Cfg"

#define PLF_L3CTL_BASE   (PLF_CLUSTER_CFG_BASE + 0x4000)

#define PLF_PLIC_BASE (UINT64_C(DT_REG_ADDR(DT_NODELABEL(plic))))
#define PLF_PLIC_SIZE   (0x2000000)
#define PLF_PLIC_ATTR   (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_PLIC_NAME   "PLIC"

#define PLF_ICCM_BASE   (UINT64_C(DT_REG_ADDR(DT_NODELABEL(iccm))))
#define PLF_ICCM_SIZE   (UINT32_C(DT_REG_SIZE(DT_NODELABEL(iccm))))
#define PLF_ICCM_ATTR   (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)
#define PLF_ICCM_NAME   "ICCMv2"

#define PLF_OCRAM_BASE    (UINT64_C(DT_REG_ADDR(DT_NODELABEL(ocram))))
#define PLF_OCRAM_SIZE    (UINT32_C(DT_REG_SIZE(DT_NODELABEL(ocram))))
#define PLF_OCRAM_ATTR    (PMP_R | PMP_W | PMP_X | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#endif /* __RISCV64_SCR9_L3_SOC_H_ */
