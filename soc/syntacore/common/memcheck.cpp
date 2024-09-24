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
 * @file memcheck.cpp
 *
 * @brief Compile-time memory map configuration checks
 *
 * @author vg-sc
 */

#pragma GCC diagnostic ignored "-Wregister"

#include <zephyr/sys/__assert.h>
#include <zephyr/arch/cpu.h>
#include <pmp.h>
#include <soc.h>
#if defined(CONFIG_SCR_MPU)
#include "../common/mpu.h"
#elif defined(CONFIG_RISCV_PMP)
#include "../common/scr_pmp.h"
#endif

#include "../common/mrt.h"
#include "../common/cluster_ctrl.h"
#include "../common/cache.h"

#ifndef PLF_MEM_MAP
#include "memory_map.h"
#endif

constexpr scr_mem_region_info mem_regions[] = { PLF_MEM_MAP };
constexpr size_t region_count = ARRAY_SIZE(mem_regions);

constexpr bool is_power_of_two(uintptr_t x) { return ((x & (x - 1)) == 0); }

/**
 * @brief Check validity of region's size and alignment
 *
 * @return true if region looks valid, false otherwise
 */
constexpr bool check_region(const scr_mem_region_info *region)
{
	return (region->size >= SCR_MEM_PROT_MIN_REGION_SIZE) && /* PMP/MPU minimal granularity requirement */
		is_power_of_two(region->size) && /* size of region must be POT */
		(region->base % region->size == 0); /* region must be NAPOT */
}

/**
 * @brief Check if region @p fst is fully overlapped by a bigger region @p snd
 *
 * @return true if @p fst is overlapped, false otherwise
 */
constexpr bool region_overlapped(const scr_mem_region_info *fst, const scr_mem_region_info *snd)
{
    /* using a trick to avoid possible overflow */
    uintptr_t fst_max = fst->size - 1 + fst->base;
    uintptr_t snd_max = snd->size - 1 + snd->base;
    return (snd->base <= fst->base && snd_max >= fst_max);
}

/**
 * @brief Check validity of MRT and PMP memory map defined by @p mem_regions
 *
 * @return true if memory map is valid, false otherwise
 */
constexpr bool check_mem_map(const scr_mem_region_info *mem_regions, size_t region_count)
{
	for (const scr_mem_region_info *p = mem_regions; p < mem_regions + region_count; ++p) {
		if (!check_region(p)) {
			return false;
                }
        }
	for (const scr_mem_region_info *low = mem_regions; low < mem_regions + region_count; ++low) {
		for (const scr_mem_region_info *high = low + 1; high < mem_regions + region_count; ++high) {
			if (region_overlapped(low, high)) {
                                /* higher-prioritized region fully overlappes a lower-prioritized one */
				return false;
                        }
                }
        }
	return true;
}

/*
 * If you struggle with debugging this assertion after modifying PLF_MEM_MAP,
 * try using the same memory map in sc-bl. The latter prints the error
 * verbosely
 */
static_assert(check_mem_map(mem_regions, region_count),
	"PLF_MEM_MAP does not satisfy some architectural requirements");
