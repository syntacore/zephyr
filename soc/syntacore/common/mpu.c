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
 * @file mpu.c
 *
 * @brief Syntacore MPU support
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
