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
 * @file mrt.h
 *
 * @brief Syntacore Memory Routing Table interface
 *
 * @author amr-sc
 */

#ifndef SCR_MRT_H
#define SCR_MRT_H

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/devicetree.h>
#include "soc.h"

/******************************************************************************
 *                              Defines section
 */
#define PLF_DEFAULT_MPORT               0

#define CLUSTER_CSR_MRT_BASE            (PLF_CLUSTER_CFG_BASE + 8 * 1024)
#define MRT_VID                         (CLUSTER_CSR_MRT_BASE)
#define MRT_ENTRY_NUM(descr)            ((descr >> 8) & 0xff)
#define MRT_CMD                         (CLUSTER_CSR_MRT_BASE + 8)
#define MRT_CMD_RUN_BIT                 (1)

#define MRT_CTRL_MTYPE_OFFS             (16)
#define MRT_CTRL_MTYPE_MASK             ~(3 << MRT_CTRL_MTYPE_OFFS)
#define MRT_CTRL_MTYPE_C_WO             (0 << MRT_CTRL_MTYPE_OFFS)
#define MRT_CTRL_MTYPE_NC_SO            (1 << MRT_CTRL_MTYPE_OFFS)
#define MRT_CTRL_MTYPE_NC_WO            (2 << MRT_CTRL_MTYPE_OFFS)
#define MRT_CTRL_MTYPE_CPU_CFG          (3 << MRT_CTRL_MTYPE_OFFS)

#define MRT_S_ENTRY_0_CTRL              (CLUSTER_CSR_MRT_BASE + 0x400 + 0 * sizeof(uint64_t))
#define MRT_S_ENTRY_0_ADDR              (CLUSTER_CSR_MRT_BASE + 0x400 + 1 * sizeof(uint64_t))
#define MRT_S_ENTRY_0_MASK              (CLUSTER_CSR_MRT_BASE + 0x400 + 2 * sizeof(uint64_t))
#define MRT_S_ENTRY_NEXT_OFFS           (0x20)
#define MRT_S_ENTRY_N_CTRL(N)           (MRT_S_ENTRY_0_CTRL + (N) * MRT_S_ENTRY_NEXT_OFFS)
#define MRT_S_ENTRY_N_ADDR(N)           (MRT_S_ENTRY_0_ADDR + (N) * MRT_S_ENTRY_NEXT_OFFS)
#define MRT_S_ENTRY_N_MASK(N)           (MRT_S_ENTRY_0_MASK + (N) * MRT_S_ENTRY_NEXT_OFFS)

#define MRT_A_ENTRY_0_CTRL              (CLUSTER_CSR_MRT_BASE + 0x800 + 0 * sizeof(uint64_t))
#define MRT_A_ENTRY_0_ADDR              (CLUSTER_CSR_MRT_BASE + 0x800 + 1 * sizeof(uint64_t))
#define MRT_A_ENTRY_0_MASK              (CLUSTER_CSR_MRT_BASE + 0x800 + 2 * sizeof(uint64_t))
#define MRT_A_ENTRY_NEXT_OFFS           (0x20)
#define MRT_A_ENTRY_N_CTRL(N)           (MRT_A_ENTRY_0_CTRL + (N) * MRT_A_ENTRY_NEXT_OFFS)
#define MRT_A_ENTRY_N_ADDR(N)           (MRT_A_ENTRY_0_ADDR + (N) * MRT_A_ENTRY_NEXT_OFFS)
#define MRT_A_ENTRY_N_MASK(N)           (MRT_A_ENTRY_0_MASK + (N) * MRT_A_ENTRY_NEXT_OFFS)

#define MRT_ENTRY_VALID                 (1)
#define CLUSTER_CSR_MPORT               (0xF000000)
#define MRT_ENTRY_BOOT_DEFAULT          (3)

#define PLF_UART0_BASE (UINT64_C(DT_REG_ADDR(DT_NODELABEL(uart))))

#define MRT_ENTRY_IO_DEFAULT            (2)
#define MRT_ENTRY_IO_ADDR               (PLF_UART0_BASE)

#define SCR_MRT_MK_ADDR64(addr) (addr)
#define SCR_MRT_MK_MASK64(size) (~((size) - 1))

#define MRT_REGION_SIZE 0x100000000ULL
#define MRT_IO_REGION_SIZE 0x10000

#if __riscv_xlen == 32
#error MRT addr/mask conversion is not implemented
#elif __riscv_xlen == 64
#define SCR_MRT_MK_ADDR(addr) SCR_MRT_MK_ADDR64(addr)
#define SCR_MRT_MK_MASK(size) SCR_MRT_MK_MASK64(size)
#else
#error MRT addr/mask conversion is not implemented
#endif

/******************************************************************************
 *                              Types section
 */
struct scr_mem_region_info_struct;
typedef struct scr_mem_region_info_struct scr_mem_region_info;

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief MRT configuration
 *
 * @param [in] mem_regions - memory regions table
 */
void scr_mrt_setup(const scr_mem_region_info *mem_regions);

/**
 * @brief Dumps MRT
 */
void scr_mrt_dump(void);

/******************************************************************************
 *                              Implementation section
 */
#if IS_ENABLED(CONFIG_SCR_MRT)
/**
 * @brief Gets number of MRT entries supported
 *
 * @return number of MRT entries supported
 */
static inline unsigned scr_mrt_num_regions(void)
{
    uint64_t descr = *(volatile uint64_t *)MRT_VID;

    return MRT_ENTRY_NUM(descr);
}

/**
 * @brief Memory region configuration
 *
 * @param [in] regn - region number
 * @param [in] base - region base address
 * @param [in] size - region size
 * @param [in] attr - region attributes
 */
static inline void scr_mrt_region_setup(unsigned regn, uintptr_t base, uint64_t size, unsigned long attr)
{
    *(volatile uint64_t *)MRT_S_ENTRY_N_MASK(regn) = SCR_MRT_MK_MASK(size);
    *(volatile uint64_t *)MRT_S_ENTRY_N_ADDR(regn) = SCR_MRT_MK_ADDR(base);
    *(volatile uint64_t *)MRT_S_ENTRY_N_CTRL(regn) = attr;

    ifence();
}

/**
 * @brief Disable memory region entry
 *
 * @param [in] regn - region number
 */
static inline void scr_mrt_region_disable(unsigned regn)
{
    *(volatile uint64_t *)MRT_S_ENTRY_N_CTRL(regn) = 0;

    ifence();
}

/**
 * @brief Apply changes made to MRT shadow registers
 */
static inline void scr_mrt_commit(void)
{
	*(volatile uint64_t *)MRT_CMD = MRT_CMD_RUN_BIT;
	while( (*(volatile uint64_t *)MRT_CMD & MRT_CMD_RUN_BIT) == 1);

	ifence();
}

#endif /* CONFIG_SCR_MRT */
#endif /* SCR_MRT_H */
