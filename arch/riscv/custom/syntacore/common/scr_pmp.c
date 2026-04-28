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
 * @file scr_pmp.c
 *
 * @brief Syntacore PMP support
 *
 * @author vdsh-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/riscv/csr.h>
#include <soc.h>
#include "scr_pmp.h"
#include "cache.h"

#if IS_ENABLED(CONFIG_RISCV_PMP)

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Calculate size of memory regions array
 */
static unsigned int get_region_count(const scr_mem_region_info *mem_regions)
{
	unsigned int i = 0;
	while (mem_regions[i].attr)
		++i;
	return i;
}

/**
 * @brief PMP initialization stub
 */
void z_riscv_pmp_init(void) {

}

/**
 * @brief Reconfigures previously configured PMP
 *
 *  PMP re-initialization process is tricky and very fragile.
 *  PMP configuration is done through a pair of CSR registers: pmpcfg and pmpaddr
 *  which couldn't be initialized atomically. First pmpaddr is configured and then
 *  pmpcfg is configured.
 *
 *  During the PMP re-initialization it is possible
 *  that new pmpaddr will be associated with the old pmpcfg value which may result in
 *  all kinds of ugly issues.
 *
 *  This function works with assumption that the addresses and the order of the
 *  previously configured regions will not be altered!
 *
 * @param [in] mem_regions - array of memory region descriptors
 */
void scr_pmp_reconfigure(const scr_mem_region_info *mem_regions)
{
	unsigned long pmp_addr[CONFIG_PMP_SLOTS * 2] = {0};
	unsigned long pmp_cfg[CONFIG_PMP_SLOTS / sizeof(unsigned long)] = {0};
	unsigned int index = 0;
	unsigned int pmp_entry_idx = get_region_count(mem_regions);
	bool is_success = true;
	const scr_mem_region_info *pmp_region;

	while (pmp_entry_idx--) {
		pmp_region = &mem_regions[pmp_entry_idx];
		is_success = is_success && z_riscv_set_pmp_entry(&index, pmp_region->attr,
			pmp_region->base,
			pmp_region->size,
			pmp_addr, pmp_cfg, ARRAY_SIZE(pmp_addr));
	}

	__ASSERT(is_success, "PMP wasn't configured");

	z_riscv_write_pmp_entries(0, index, true, pmp_addr, pmp_cfg);
}

/**
 * @brief PMP configuration
 *
 * PMP entries are configured in reverse order compared to @p mem_regions, so
 * the last entry in @p mem_regions is the most prioritized
 *
 * @param [in] mem_regions - array of memory region descriptors
 */
void scr_pmp_setup(const scr_mem_region_info *mem_regions)
{
	/* Revert previous configuration */
	scr_pmp_deinit();
	/* Re-init PMP from scratch */
	scr_pmp_reconfigure(mem_regions);
}

#endif /* CONFIG_RISCV_PMP */
