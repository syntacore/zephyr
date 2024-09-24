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
 * @file soc.c
 *
 * @brief Syntacore SoC initialization
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
#include <pmp.h>
#include <soc.h>
#if IS_ENABLED(CONFIG_SCR_MPU)
#include "../../common/mpu.h"
#elif IS_ENABLED(CONFIG_RISCV_PMP)
#include "../../common/scr_pmp.h"
#endif
#include "../../common/cache.h"
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
 * @brief Perform Primary core initialization at boot time
 *
 * @return 0
 */
int riscv_init_primary(void)
{
	/* Early PMP/MPU initialization - bring PMP/MPU to a known state */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_early_init();
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_early_init();
#endif

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
	/* Final PMP/MPU initialization */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_setup(mem_regions);
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_setup(mem_regions);
#endif

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
	/* Bring-up L1 */
	if (IS_ENABLED(CONFIG_SCR_L1_CACHE)) {
		scr_cache_l1_enable();
	}
	/* Final PMP/MPU initialization */
#if IS_ENABLED(CONFIG_RISCV_PMP)
	scr_pmp_setup(mem_regions);
#elif IS_ENABLED(CONFIG_SCR_MPU)
	scr_mpu_setup(mem_regions);
#endif
}
#endif /* CONFIG_MP_MAX_NUM_CPUS > 1 */

SYS_INIT(riscv_init_primary, EARLY, 0);
