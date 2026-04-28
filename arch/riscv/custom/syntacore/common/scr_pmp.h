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
 * @file scr_pmp.h
 *
 * @brief Syntacore PMP interface
 *
 * @author vdsh-sc
 */

#ifndef SCR_PMP_H
#define SCR_PMP_H

/******************************************************************************
 *                              Includes section
 */
#ifndef _LINKER
#include <zephyr/devicetree.h>
#include <pmp.h>
#endif /* _LINKER */
/******************************************************************************
 *                              Defines section
 */
#define SCR_PMP_CTRL_MT_WEAKLY      (0 << 5)
#define SCR_PMP_CTRL_MT_STRONG      (1 << 5)
#define SCR_PMP_CTRL_MT_WEAKLY_NC   (2 << 5)
#define SCR_PMP_CTRL_MT_CONFIG      (3 << 5)

/* Minimal PMP region size in bytes (4KB according to scr9 spec) */
#define SCR_MEM_PROT_MIN_REGION_SIZE 4096

/******************************************************************************
 *                              Types section
 */
#ifndef _LINKER
/* platform memory configuration */
typedef struct scr_mem_region_info_struct {
    uintptr_t      base;
    uintptr_t      size;
    unsigned long  attr;
    const char    *name;
    unsigned long  mrt_attr;
} scr_mem_region_info;

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief PMP configuration
 *
 * PMP entries are configured in reverse order compared to @p mem_regions, so
 * the last entry in @p mem_regions is the most prioritized
 *
 * @param [in] mem_regions - array of memory region descriptors
 */
void scr_pmp_setup(const scr_mem_region_info *mem_regions);

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
void scr_pmp_reconfigure(const scr_mem_region_info *mem_regions);

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Early PMP initialization. Brings PMP to known state after possible
 *		warm reset
 */
static ALWAYS_INLINE void scr_pmp_early_init(void)
{
	unsigned long pmp_addr[4] = {0};
	unsigned long pmp_cfg = 0;
	unsigned int index = 0;

	bool is_success = z_riscv_set_pmp_entry(&index, PMP_R | PMP_W | SCR_PMP_CTRL_MT_CONFIG,
		PLF_MMCFG_BASE,
		PLF_MMCFG_SIZE,
		pmp_addr, &pmp_cfg, ARRAY_SIZE(pmp_addr));

	is_success = is_success && z_riscv_set_pmp_entry(&index, PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG,
		PLF_MMIO_BASE,
		PLF_MMIO_SIZE,
		pmp_addr, &pmp_cfg, ARRAY_SIZE(pmp_addr));

	__ASSERT(is_success, "PMP wasn't configured");

	z_riscv_write_pmp_entries(0, index, true, pmp_addr, &pmp_cfg);
}

/**
 * @brief PMP deinitialization
 */
static ALWAYS_INLINE void scr_pmp_deinit(void)
{
	csr_write(pmpcfg0, 0);
	if (!IS_ENABLED(CONFIG_64BIT)) {
		csr_write(pmpcfg1, 0);
	}
	csr_write(pmpcfg2, 0);
	if (!IS_ENABLED(CONFIG_64BIT)) {
		csr_write(pmpcfg3, 0);
	}
}

#endif /* _LINKER */
#endif /* SCR_PMP_H */
