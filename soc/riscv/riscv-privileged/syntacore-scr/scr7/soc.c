/*
 * @file soc.c
 *
 * @brief Syntacore SoC initialization
 *
 * @author dz-sc
 *
 * Copyright (c) 2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <soc.h>
#include "../common/mpu.h"
#include "../common/cache.h"

/******************************************************************************
 *                              Static variables section
 */
#if IS_ENABLED(CONFIG_SCR_MPU)
static const scr_mem_region_info mem_regions[] = {
    PLF_MEM_MAP,
    {0, 0, 0, 0}
};
#endif /* CONFIG_SCR_MPU */

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
	/* Early MPU initialization - bring MPU to a known state */
	if (IS_ENABLED(CONFIG_SCR_MPU)) {
		scr_mpu_early_init();
	}
	/* Bring-up caches */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE) || IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
		if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
			scr_cache_l1_disable();
		}
		if (IS_ENABLED(CONFIG_SCR_L2_CACHE)) {
			scr_cache_l2_enable();
		}
		if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
			scr_cache_l1_enable();
		}
	}
	/* Final MPU initialization */
	if (IS_ENABLED(CONFIG_SCR_MPU)) {
		scr_mpu_setup(mem_regions);
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
NO_STACK_PROTECTOR void riscv_early_init_secondary(void)
{
	/* Bring MPU and Caches to initial state: cache-bypass, L1 disabled  */
	if (IS_ENABLED(CONFIG_SCR_MPU)) {
		scr_mpu_early_init();
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
	/* Bring-up L1 */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_enable();
	}
	/* Final MPU initialization */
	if (IS_ENABLED(CONFIG_SCR_MPU)) {
		scr_mpu_setup(mem_regions);
	}
}
#endif /* CONFIG_MP_MAX_NUM_CPUS > 1 */

SYS_INIT(riscv_init_primary, EARLY, 0);
