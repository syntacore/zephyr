/*
 * @file mpu.c
 *
 * @brief Syntacore MPU support
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
#include <zephyr/arch/riscv/csr.h>
#include <soc.h>
#include "mpu.h"
#include "cache.h"

#if IS_ENABLED(CONFIG_SCR_MPU)

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief MPU configuration
 *
 * @param [in] mem_regions - array of memory region descriptors
 */
void scr_mpu_setup(const scr_mem_region_info *mem_regions)
{
	/* configure all regions */
	const scr_mem_region_info *rgn = mem_regions;
	unsigned rn = 1;
	while (rgn->size && rn < SCR_MPU_REGIONS) {
		if (rgn->attr) {
			scr_mpu_region_setup(rn, rgn->base, rgn->size, rgn->attr | SCR_MPU_CTRL_VALID);
			ifence();
			++rn;
		}
		++rgn;
	}

	if (rn > 1) {
		/* disable all unused regions, disable default region #0 */
		do {
			csr_write(SCR_CSR_MPU_SEL, rn++);
			csr_write(SCR_CSR_MPU_CTRL, 0);
			ifence();
		} while (rn < SCR_MPU_REGIONS);

		csr_write(SCR_CSR_MPU_SEL, 0);
		csr_write(SCR_CSR_MPU_CTRL, 0);
	}
}

#endif /* CONFIG_SCR_MPU */
