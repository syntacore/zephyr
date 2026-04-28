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
 * @file mrt.c
 *
 * @brief Syntacore Memory Routing Table support
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include "mrt.h"

#if IS_ENABLED(CONFIG_SCR_MPU)
#include "mpu.h"
#endif /* CONFIG_SCR_MPU */

#if IS_ENABLED(CONFIG_RISCV_PMP)
#include "scr_pmp.h"
#endif /* CONFIG_RISCV_PMP */

#if IS_ENABLED(CONFIG_SCR_MRT)

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief MRT configuration
 *
 * @param [in] mem_regions - memory regions table
 */
void scr_mrt_setup(const scr_mem_region_info *mem_regions)
{
    /* Configure all regions */
    const scr_mem_region_info *rgn = mem_regions;
    /*
     * Assuming cluster configuration registers are at pre-configured region 0
     * and in PLF_MEM_MAP entry 0
     */
    ++rgn;
    unsigned rn = 1;
    while (rgn->size) {
        if (rgn->attr) {
            scr_mrt_region_setup(rn, rgn->base, rgn->size, (rgn->mrt_attr | MRT_ENTRY_VALID));
            ++rn;
        }
        ++rgn;
    }

    /* Disable unused regions */
    int rmax = scr_mrt_num_regions();
    while (rn < rmax) {
        scr_mrt_region_disable(rn);
        ++rn;
    }

    scr_mrt_commit();
}

/**
 * @brief Dumps MRT
 */
void scr_mrt_dump(void)
{
    unsigned rmax = scr_mrt_num_regions();

    printk("MRT regions:%u\n", rmax);
    printk("#    CTRL		BASE		MASK\n");

    for (unsigned rn = 0; rn < rmax; ++rn) {
        const uint64_t ctrl = *(volatile uint64_t *)MRT_A_ENTRY_N_CTRL(rn);
        if (ctrl & MRT_ENTRY_VALID) {
            const uint64_t base =  *(volatile uint64_t *)MRT_A_ENTRY_N_ADDR(rn);
            const uint64_t mask =  *(volatile uint64_t *)MRT_A_ENTRY_N_MASK(rn);

            printk("%u   0x%08llX   0x%016llX   0x%016llX\n", rn, ctrl, base, mask);
        }
    }
}

#endif /* CONFIG_SCR_MRT */
