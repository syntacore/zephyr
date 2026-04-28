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
 * @file main.c
 *
 * @brief Syntacore Cache flush/invalidate Test #3
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <soc.h>
#include <zephyr/cache.h>
#include <memory_map.h>
#include <scr_pmp.h>

/******************************************************************************
 *                              Defines section
 */
#define STACK_SIZE	(8 * 1024)

#define CORES_NUM	CONFIG_MP_MAX_NUM_CPUS

#define TEST_REGION_SIZE (512 * 1024)
#define TEST_REGION_ADDR PLF_DDR2_BASE
#define TEST_TRANSACTIONS_COUNT 1000
#define TEST_SEQUENCE_FRAGMENT_SIZE 16
BUILD_ASSERT(TEST_REGION_SIZE >= CORES_NUM, "Invalid test region size");

#define PLF_TEST_MEM_ATTR_CACHED (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_TEST_MEM_ATTR_UNCACHED (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY_NC)

#define TEST_REGION_MRT_ENTRY \
	{PLF_DDR2_BASE, TEST_REGION_SIZE * CORES_NUM, PLF_MEM_ATTR, "Test", PLF_DEFAULT_MPORT}

/* Test region index is the first in the memory regions map */
#define TEST_REGION_MAP_INDEX (0)

#define POISON_PATTERN 0x5A5A5A5A5A5A5A5AULL
#define SPARE_POISON_PATTERN 0xF0F0F0F0F0F0F0F0ULL

enum caching_mode {
	CACHING_MODE_OFF = 0,
	CACHING_MODE_ON,
};

/******************************************************************************
 *                              Static variables section
 */
static K_THREAD_STACK_ARRAY_DEFINE(tstack, CORES_NUM, STACK_SIZE);
static struct k_thread tthread[CORES_NUM];
static uint32_t transactions_remaining = TEST_TRANSACTIONS_COUNT;
static struct k_sem start_sync, completion_sync;

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Get thread dedicated to particular CPU
 *
 * @param [in] core_id - CPU core ID
 *
 * @return thread dedicated to particular CPU
 */
static inline struct k_thread *get_thread(uint8_t core_id)
{
	return &tthread[core_id];
}

/**
 * @brief Get current pattern
 *
 * @return current pattern
 */
static inline uint8_t get_pattern(uint8_t seed)
{
	 return (transactions_remaining & 0xFF) + seed;
}

/**
 * @brief Gets poison pattern
 *
 * @return poison pattern
 */
static inline uint64_t get_poison(void)
{
	/* Use alternative poison to avoid collision with the current pattern */
	for (uint8_t cpu_id = 0; cpu_id < CORES_NUM; cpu_id++) {
		if (get_pattern(cpu_id) == (uint8_t)POISON_PATTERN) {
			return SPARE_POISON_PATTERN;
		}
	}

	return POISON_PATTERN;
}

/**
 * @brief Fills memory with pattern provided
 *
 * @param [in] start - start address of memory to fill
 * @param [in] pattern - pattern to use
 * @param [in] size - memory size
 */
static void fill_pattern(uint64_t *start, uint64_t pattern, size_t size)
{
	for (uint64_t *ptr = start; (uintptr_t)ptr < (uintptr_t)start + size; ptr++) {
		*ptr = pattern;
	}
}

/**
 * @brief Per-CPU caching mode configuration
 *
 * @param [in] mode - caching mode
 */
static void configure_caching(enum caching_mode mode)
{
	if (IS_ENABLED(CONFIG_RISCV_PMP)) {
		scr_mem_region_info regions_map[] = {
			/* Test region placeholder */
			{0, 0, 0, 0, 0},
			PLF_MEM_MAP,
			/* Terminator */
			{0, 0, 0, 0, 0}
		};
		scr_mem_region_info test_region_info = {
			.base = TEST_REGION_ADDR,
			.size = TEST_REGION_SIZE,
			.name = "Test",
			.mrt_attr = PLF_DEFAULT_MPORT
		};

		if (mode == CACHING_MODE_OFF) {
			test_region_info.attr = PLF_TEST_MEM_ATTR_UNCACHED;
		} else {
			test_region_info.attr = PLF_TEST_MEM_ATTR_CACHED;
		}

		memcpy(&regions_map[TEST_REGION_MAP_INDEX],
			&test_region_info, sizeof(test_region_info));
		scr_pmp_reconfigure(regions_map);
	}
}

/**
 * @brief Verifies pattern in memory
 *
 * @param [in] mode - caching mode in which memory is to be verified
 */
static void verify_memory(enum caching_mode mode)
{
	uint8_t *ptr = (uint8_t *)TEST_REGION_ADDR;

	/* Disable caching when requested */
	if (mode == CACHING_MODE_OFF) {
		configure_caching(CACHING_MODE_OFF);
	}

	while ((uintptr_t)ptr < TEST_REGION_ADDR + TEST_REGION_SIZE)
	{
		for (uint8_t next_core_id = 0; next_core_id < CORES_NUM; next_core_id++) {
			uint8_t next_pattern = get_pattern(next_core_id);

			if (ptr[next_core_id] != next_pattern) {
				printk("Verif core%d[%p]:%x|%x cached:%d\n",
					next_core_id, &ptr[next_core_id],
					ptr[next_core_id], next_pattern, mode);
				__asm__ volatile("ebreak");
			}
		}

		uint64_t poison = get_poison();
		for (uint8_t *poison_ptr = ptr + CORES_NUM;
				poison_ptr < ptr + TEST_SEQUENCE_FRAGMENT_SIZE; poison_ptr++) {
			if (*poison_ptr != (uint8_t)poison) {
				printk("Non-poison[%p]:%x cached:%d\n", poison_ptr, *poison_ptr, mode);
				__asm__ volatile("ebreak");
			}
		}

		ptr += TEST_SEQUENCE_FRAGMENT_SIZE;
	}

	/* Turn back the caching */
	if (mode == CACHING_MODE_OFF) {
		configure_caching(CACHING_MODE_ON);
	}
}

/**
 * @brief Invalidate the entire test region
 */
static inline void invalidate_test_region(void)
{
	uint8_t *ptr = (uint8_t *)TEST_REGION_ADDR;

	cache_data_invd_range(ptr, TEST_REGION_SIZE);
}

/**
 * @brief Test procedure: verify shared memory region and update
 * 		it with new pattern
 *
 * @param [in] core_id - CPU core id
 */
static void test_action(uint8_t core_id)
{
	uint8_t pattern = get_pattern(core_id);

	/* Invalidate memory region to receive possibly new poison pattern */
	invalidate_test_region();

	/*
	 * Before each transaction fill test region with poison using
	 * core 0 in NC mode, then release all the cores
	 */
	if (core_id == 0) {
		uint64_t *ptr = (uint64_t *)TEST_REGION_ADDR ;
		uint64_t poison = get_poison();

		configure_caching(CACHING_MODE_OFF);
		fill_pattern(ptr, poison, TEST_REGION_SIZE);
		configure_caching(CACHING_MODE_ON);

		k_sem_give(&start_sync);
	}

	/* Synchronize all the cores */
	while (k_sem_count_get(&start_sync) == 0)
		;

	/* Each core fills the test region with a stride */
	uint8_t *ptr = (uint8_t *)TEST_REGION_ADDR + core_id;
	while ((uintptr_t)ptr < TEST_REGION_ADDR + TEST_REGION_SIZE)
	{
		*ptr = pattern;
		cache_data_flush_range(ptr, TEST_SEQUENCE_FRAGMENT_SIZE);
		ptr += TEST_SEQUENCE_FRAGMENT_SIZE;
	}

	/* Check pass 1: check only self-written bytes */
	ptr = (uint8_t *)TEST_REGION_ADDR + core_id;
	while ((uintptr_t)ptr < TEST_REGION_ADDR + TEST_REGION_SIZE)
	{
		if (*ptr != pattern) {
			printk("On core%d[%p]:%x|%x\n", core_id, ptr, *ptr, pattern);
			__asm__ volatile("ebreak");
		}
		ptr += TEST_SEQUENCE_FRAGMENT_SIZE;
	}

	if (core_id == 0) {
		/* Wait till all the cores complete the transaction */
		while (k_sem_count_get(&completion_sync) != (CORES_NUM - 1))
			;

		/* Check pass 2: core 0 verifies the entire region in Cached mode */
		verify_memory(CACHING_MODE_ON);
		/* Check pass 3: core 0 verifies the entire region in Uncached mode */
		verify_memory(CACHING_MODE_OFF);
	}
}

/**
 * @brief MRT initialization
 */
static void init_mrt(void)
{
	if (IS_ENABLED(CONFIG_SCR_MRT)) {
		const scr_mem_region_info mem_regions[] = {
			PLF_MEM_MAP,
			TEST_REGION_MRT_ENTRY,
			{0, 0, 0, 0, 0}
		};

		scr_mrt_setup(mem_regions);
	}
}

/**
 * @brief Per-CPU initialization
 *
 * @param [in] core_id - CPU core id
 */
static void init_per_cpu(uint8_t core_id)
{
	configure_caching(CACHING_MODE_ON);
}

/**
 * @brief Execution thread
 *
 * @param [in] arg1 - thread argument 1
 * @param [in] arg2 - thread argument 2
 * @param [in] arg3 - thread argument 3
 */
static void test_thread(void *arg1, void *arg2, void *arg3)
{
	int core_id = (uintptr_t)arg1;

	init_per_cpu(core_id);

	/* Run main test */
	while (transactions_remaining) {
		while (0 != k_sem_count_get(&completion_sync))
			;

		if (!transactions_remaining)
			return;

		test_action(core_id);

		k_sem_give(&completion_sync);

		if (core_id == 0) {
			transactions_remaining--;
			k_sem_reset(&start_sync);
			k_sem_reset(&completion_sync);
		}
	}
}

/**
 * @brief Test initialization
 */
static void test_init(void)
{
	init_mrt();

	memset(tthread, 0x00, sizeof(tthread));
	memset(tstack, 0x00, sizeof(tstack));
	k_sem_init(&completion_sync, 0, CORES_NUM);
	k_sem_init(&start_sync, 0, 1);

	for (uintptr_t core_id = 0; core_id < CORES_NUM; core_id++) {
		struct k_thread *thread = get_thread(core_id);

		k_thread_create(thread, tstack[core_id], STACK_SIZE,
		   (k_thread_entry_t)test_thread,
		   (void *)core_id, NULL, NULL,
		   K_PRIO_COOP(10), 0, K_FOREVER);

		k_thread_cpu_pin(thread, core_id);
	}
}

/**
 * @brief Test entry point
 */
int main(void)
{
	test_init();

	/* Launching all the threads */
	for (uintptr_t core_id = 0; core_id < CORES_NUM; core_id++) {
		struct k_thread *thread = get_thread(core_id);

		k_thread_start(thread);
	}
	/* Waiting for all the threads to complete */
	for (uintptr_t core_id = 0; core_id < CORES_NUM; core_id++) {
		struct k_thread *thread = get_thread(core_id);

		k_thread_join(thread, K_FOREVER);
	}

	printk("%d transactions completed on %d cores!\n",
		TEST_TRANSACTIONS_COUNT, CORES_NUM);

	return 0;
}
