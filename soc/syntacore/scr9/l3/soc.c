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
 * @file soc.c
 *
 * @brief SCR9 L3 SoC initialization
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <soc.h>
#include <memory_map.h>
#include <pmp.h>
#include "../../common/cache.h"
#include "../../common/mrt.h"
#include "../../common/cluster_ctrl.h"
#include "../../common/scr_pmp.h"
#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
#include <zephyr/linker/linker-defs.h>
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */

/******************************************************************************
 *                              Static variables section
 */
#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
extern char __unused_mem_size[];
extern char __unused_mem_start[];
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */

#if IS_ENABLED(CONFIG_SCR_MRT)
static const scr_mem_region_info mem_regions[] = {
  	PLF_MEM_MAP,
#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
	{(uintptr_t) __rom_region_start, (uintptr_t) __rom_region_size, PMP_R | PMP_X | PMP_L | SCR_PMP_CTRL_MT_WEAKLY, PLF_MEM_NAME, PLF_DEFAULT_MPORT},
	{(uintptr_t) __unused_mem_start, (uintptr_t) __unused_mem_size, PMP_L | SCR_PMP_CTRL_MT_WEAKLY, PLF_MEM_NAME, PLF_DEFAULT_MPORT}, 
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */
	{0, 0, 0, 0, 0}
};
#endif /* CONFIG_SCR_MRT */

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Perform Primary core initialization at boot time
 *
 * @return 0
 */
int riscv_init_primary(void)
{
	/* Early PMP initialization - bring PMP to a known state */
	if (IS_ENABLED(CONFIG_RISCV_PMP)) {
		scr_pmp_early_init();
	}

#if IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER)
	/* Init L1D prefetcher */
	scr_cache_l1d_prefetcher_init();
#endif /* IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER) */

	/* Bring-up caches */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE) || IS_ENABLED(CONFIG_SCR_L2_CACHE) ||
			IS_ENABLED(CONFIG_SCR_L3_CACHE)) {
		if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
			scr_cache_l1_disable();
		}
		if (IS_ENABLED(CONFIG_SCR_L3_CACHE)) {
			scr_cache_l3_init();
		}
		if (IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
			scr_cache_l2_enable();
		}
		if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
			scr_cache_l1_enable();
		}
	}
	/* Final PMP initialization */
	if (IS_ENABLED(CONFIG_RISCV_PMP)) {
		scr_pmp_setup(mem_regions);
	}

	/* MRT initialization */
	if (IS_ENABLED(CONFIG_SCR_MRT)) {
		scr_mrt_setup(mem_regions);
	}

	return 0;
}

#if CONFIG_MP_MAX_NUM_CPUS > 1
#if IS_ENABLED(CONFIG_RISCV_EARLY_INIT_SECONDARY)
/**
 * @brief Perform Secondary core early initialization.
 *
 * 		Stack protection checks generation should be disabled within the
 * 		entire scope of this function to avoid race condition with
 * 		Primary core. This means that all called functions should be
 * 		either inlined or marked with NO_STACK_PROTECTOR
 */

/* FIXME! Stack protection is not supported for PMP SMP yet */
NO_STACK_PROTECTOR void riscv_early_init_secondary(void)
{
	/* Bring MPU and Caches to initial state: cache-bypass, L1 disabled  */
	if (IS_ENABLED(CONFIG_RISCV_PMP)) {
		scr_pmp_early_init();
	}
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_disable();
	}
}

#endif /* CONFIG_RISCV_EARLY_INIT_SECONDARY */

/**
 * @brief Perform Secondary core later-stage initialization
 */
void riscv_init_secondary(void)
{
#if IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER)
	/* Init L1D prefetcher */
	scr_cache_l1d_prefetcher_init();
#endif /* IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER) */

	/* Bring-up L1 */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_enable();
	}

	/* Bring-up L2 */
	if (IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
		scr_cache_l2_enable();
	}

	/* Final PMP initialization */
	if (IS_ENABLED(CONFIG_RISCV_PMP)) {
		scr_pmp_setup(mem_regions);
	}
}
#endif /* CONFIG_MP_MAX_NUM_CPUS > 1 */

SYS_INIT(riscv_init_primary, EARLY, 0);
