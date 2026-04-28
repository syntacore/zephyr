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
 * @file cache.c
 *
 * @brief Syntacore L1/L2/L3 cache support
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
#include "cache.h"

/******************************************************************************
 *                              Defines section
 */
#define ENC_CLINV(regn) (0x10800073 | (((regn) & 0x1f) << 15))
#define ENC_CLFLUSH(regn) (0x10900073 | (((regn) & 0x1f) << 15))

/******************************************************************************
 *                              Static variables section
 */
#if IS_ENABLED(CONFIG_SCR_L3_CACHE)
static struct k_spinlock l3_lock;
#endif /* IS_ENABLED(CONFIG_SCR_L3_CACHE) */

/******************************************************************************
 *                              Implementation section
 */
#if IS_ENABLED(CONFIG_SCR_L1_CACHE) || IS_ENABLED(CONFIG_SCR_L2_CACHE) || IS_ENABLED(CONFIG_SCR_L3_CACHE)
#if IS_ENABLED(CONFIG_SCR_L2_CACHE)

/**
 * @brief Enables L2 cache
 */
void scr_cache_l2_enable(void)
{
	volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

	/* Cache doesn't exist or not supported */
	if (!l2ctl[L2_CSR_VER_IDX]) {
		return;
	}

	uint32_t l2desc0 = l2ctl[L2_CSR_DESCR_IDX];
	uint32_t cbmask = (1 << SCR_L2_DESCR_BANKS(l2desc0)) - 1;
	uint32_t external_coherence = 0;

	if (IS_ENABLED(CONFIG_SCR_L3_CACHE)) {
		external_coherence = 1;
	}

	/* Disable L2$ */
	l2ctl[L2_CSR_EN_IDX] = 0;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Configure external coherence */
	l2ctl[L2_CSR_SYSCO_IDX] = external_coherence;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Invalidate */
	l2ctl[L2_CSR_INV_IDX] = cbmask;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Enable L2$ */
	l2ctl[L2_CSR_EN_IDX] = cbmask;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
}

/**
 * @brief Disables L2 cache
 */
void scr_cache_l2_disable(void)
{
	volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

	/* Cache doesn't exist or not supported */
	if (!l2ctl[L2_CSR_VER_IDX]) {
		return;
	}

	/* Disable L2$ */
	l2ctl[L2_CSR_EN_IDX] = 0;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Disable external coherence */
	l2ctl[L2_CSR_SYSCO_IDX] = 0;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Flush overall */
	l2ctl[L2_CSR_FLUSH_IDX] = ~0U;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
	/* Invalidate L2$ */
	l2ctl[L2_CSR_INV_IDX] = ~0U;
	/* Confirm state */
	while (l2ctl[L2_CSR_BUSY_IDX]);
}

/**
 * @brief Checks if L2 cache is enabled
 *
 * @return true - if L2 cache is enabled; false - otherwise
 */
bool scr_cache_l2_is_enabled(void)
{
	volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

	return l2ctl[L2_CSR_VER_IDX] && l2ctl[L2_CSR_EN_IDX];
}

#endif /* IS_ENABLED(CONFIG_SCR_L2_CACHE) */

#if IS_ENABLED(CONFIG_SCR_L3_CACHE)
/**
 * @brief Gets L3 cache banks count
 *
 * @return L3 cache banks count
 */
static inline uint8_t scr_cache_l3_get_banks_count(void)
{
	volatile uint64_t l3_desc = *(uint64_t *)L3C_DESCR_CACHE;

	return (l3_desc >> L3C_DESCR_CACHE_OFFS_BANK_NUM) &
		L3C_DESCR_CACHE_MASK_BANK_NUM;
}

/**
 * @brief Waits for CMO completion
 */
static void scr_cache_l3_wait_cmo_completion(void)
{
	uint8_t banks_count = scr_cache_l3_get_banks_count();

	for (uint8_t bank = 0; bank < banks_count; bank++) {
		volatile uint64_t *l3b_cmd_sts = (uint64_t *)L3B_CMD_STS_N(bank);

		while (((*l3b_cmd_sts >> L3B_CMD_STS_OFFS_DONE) & L3B_CMD_STS_MASK_DONE) == 0)
			;
	}
}

/**
 * @brief Runs Cache Management Operation (CMO)
 *
 * @param [in] cmo - CMO representation: value to be written to L3C_CMD_CTRL
 */
static void scr_cache_l3_run_cmo(uint64_t cmo)
{
	volatile uint64_t *l3ctl = (uint64_t *)PLF_L3CTL_BASE;

	K_SPINLOCK(&l3_lock) {
		/* Start CMO */
		l3ctl[L3C_CMD_CTRL_IDX] = cmo;
		/* Wait for completion */
		scr_cache_l3_wait_cmo_completion();
	}
}

/**
 * @brief L3 cache initialization
 */
void scr_cache_l3_init(void)
{
	volatile uint64_t vid = *(uint64_t *)L3C_VID;

	/* Cache doesn't exist or not supported */
	if (!vid) {
		return;
	}

	/*
	 * This CMO is triggered without L3 spinlock since at this
	 * point the caches are not yet enabled and the atomic operations
	 * couldn't be used
	 */
	volatile uint64_t *l3ctl = (uint64_t *)PLF_L3CTL_BASE;

	l3ctl[L3C_CMD_CTRL_IDX] = L3C_CMD_INVALIDATE_ALL;
	/* Wait for completion */
	scr_cache_l3_wait_cmo_completion();
}

/**
 * @brief L3 cache flush
 */
void scr_cache_l3_flush(void)
{
	scr_cache_l3_run_cmo(L3C_CMD_FLUSH_ALL);
}

#endif /* IS_ENABLED(CONFIG_SCR_L3_CACHE) */

#if IS_ENABLED(CONFIG_SCR_CUSTOM_CACHE_INSTRUCTIONS)
/**
 * @brief D-cache address range invalidate
 *
 * @param [in] addr - range start address
 * @param [in] size - range size
 */
void dcache_invalidate(uintptr_t addr, long size)
{
    fence();

    if (size) {
        register uintptr_t a0 = addr;
        do {
        	__asm__ volatile(".equ code_clinv, "STRINGIFY(ENC_CLINV(10))";"
                         "mv a0, %0; .word code_clinv;" ::"r"(a0) : "a0");
            a0 += CONFIG_DCACHE_LINE_SIZE;
            size -= CONFIG_DCACHE_LINE_SIZE;
        } while (size > 0);
    }
}

/**
 * @brief D-cache address range flush. Invalidation will also take place after the flush.
 *
 * @param [in] addr - range start address
 * @param [in] size - range size
 */
void dcache_flush(uintptr_t addr, long size)
{
    if (size) {
        register uintptr_t a0 = addr;
        /* Flush the cache for the requested range */
        do {
        	__asm__ volatile(".equ code_clflush, "STRINGIFY(ENC_CLFLUSH(10))";"
                         "mv a0, %0; .word code_clflush;" ::"r"(a0) : "a0");
            a0 += CONFIG_DCACHE_LINE_SIZE;
            size -= CONFIG_DCACHE_LINE_SIZE;
        } while (size > 0);
    }

    fence();
}
#endif /* IS_ENABLED(CONFIG_SCR_CUSTOM_CACHE_INSTRUCTIONS) */
#endif /* IS_ENABLED(CONFIG_SCR_L1_CACHE) || IS_ENABLED(CONFIG_SCR_L2_CACHE) || IS_ENABLED(CONFIG_SCR_L3_CACHE)*/
