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
 * @file soc.c
 *
 * @brief Syntacore SoC initialization
 *
 * @author dz-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <pmp.h>
#include <soc.h>
#include "plic.h"
#include "mtimer.h"
#if IS_ENABLED(CONFIG_SCR_PMA)
#include "pma.h"
#endif /* CONFIG_SCR_PMA */
#if IS_ENABLED(CONFIG_SCR_MPU)
#include "mpu.h"
#elif IS_ENABLED(CONFIG_RISCV_PMP)
#include "scr_pmp.h"
#endif
#if IS_ENABLED(CONFIG_SCR_MRT)
#include "mrt.h"
#include <memory_map.h>
#endif
#include "cache.h"
#include "generic_cache.h"
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

static const scr_mem_region_info mem_regions[] = {
	PLF_MEM_MAP,
#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
	{(uintptr_t) __rom_region_start, (uintptr_t) __rom_region_size, PMP_R | PMP_X | PMP_L | SCR_PMP_CTRL_MT_WEAKLY, PLF_MEM_NAME},
	{(uintptr_t) __unused_mem_start, (uintptr_t) __unused_mem_size, PMP_L | SCR_PMP_CTRL_MT_WEAKLY, PLF_MEM_NAME},
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */
	{0, 0, 0, 0, 0}
};

/******************************************************************************
 *                              Implementation section
 */

/**
 * @brief Perform Primary core caches initialization
 */
void arch_cache_init(void)
{
#if IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER)
	/* Init L1D prefetcher */
	scr_cache_l1d_prefetcher_init();
#endif /* IS_ENABLED(CONFIG_SCR_L1D_PREFETCHER) */

	/* Bring-up caches */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_disable();
	}

	if (IS_ENABLED(CONFIG_SCR_L3_GENERIC_CACHE)) {
		gc_init(GC_L3);
	} else if (IS_ENABLED(CONFIG_SCR_L3_CACHE)) {
		scr_cache_l3_init();
	}

	if (IS_ENABLED(CONFIG_SCR_L2_GENERIC_CACHE)) {
		gc_init(GC_L2);
	} else if (IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
		scr_cache_l2_enable();
	}

	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_enable();
	}
}

/**
 * @brief SoC hook executed after the reset vector.
 *
 * Perform Primary core initialization at boot time
 */
void soc_prep_hook(void)
{
	/* Early PMA/PMP/MPU initialization - bring PMA/PMP/MPU to a known state */
#if IS_ENABLED(CONFIG_SCR_PMA)
	scr_pma_early_init();
#endif /* CONFIG_SCR_PMA */

#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_early_init();
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_early_init();
#endif
}

/*
 * @brief SoC hook executed before the kernel and devices are initialized.
 *
 * Performs remaining early initialization actions
 */
void soc_early_init_hook(void)
{
	/* Final PMP/MPU initialization */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_setup(mem_regions);
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_setup(mem_regions);
#endif

	/* MRT initialization */
#if IS_ENABLED(CONFIG_SCR_MRT)
	scr_mrt_setup(mem_regions);
#endif
#if IS_ENABLED(CONFIG_SCR_PMA)
	scr_pma_setup();
#endif /* CONFIG_SCR_PMA */

	mtimer_early_init();
}

/*
 * @brief SoC hook executed after the kernel and devices are initialized
 */
void soc_late_init_hook(void)
{
#if IS_ENABLED(CONFIG_RISCV_HAS_PLIC)
	plic_late_init();
#endif
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
NO_STACK_PROTECTOR void riscv_early_init_secondary(void)
{
#if IS_ENABLED(CONFIG_SCR_PMA)
	scr_pma_early_init();
#endif /* CONFIG_SCR_PMA */

	/* Bring PMP/MPU and Caches to initial state: cache-bypass, L1 disabled  */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_early_init();
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_early_init();
#endif

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

	/* Bring-up L2 in case of L3 */
	if (IS_ENABLED(CONFIG_SCR_L3_CACHE) || IS_ENABLED(CONFIG_SCR_L3_GENERIC_CACHE)) {
		if (IS_ENABLED(CONFIG_SCR_L2_GENERIC_CACHE)) {
			gc_init(GC_L2);
		} else if (IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
			scr_cache_l2_enable();
		}
	}

	/* Final PMP/MPU initialization */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_setup(mem_regions);
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_setup(mem_regions);
#endif

#if IS_ENABLED(CONFIG_SCR_PMA)
	scr_pma_setup();
#endif /* CONFIG_SCR_PMA */
}
#endif /* CONFIG_MP_MAX_NUM_CPUS > 1 */
