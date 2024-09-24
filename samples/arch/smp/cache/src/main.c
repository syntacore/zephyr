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
 * @brief Syntacore Cache flush/invalidate Test
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

#define CORES_NUM	CONFIG_MP_NUM_CPUS
#define BOUND_CORES_NUM 2
BUILD_ASSERT(CORES_NUM % BOUND_CORES_NUM == 0, "Even number of cores expected");

#define TEST_REGION_ADDR(cpu_id) (PLF_DDR2_BASE + (cpu_id / BOUND_CORES_NUM) * TEST_REGION_SIZE)
#define TEST_REGION_SIZE 4096
#define TEST_TRANSACTIONS_COUNT 18000000

/* Test region index is the first in the memory regions map */
#define TEST_REGION_MAP_INDEX (0)

#define PLF_TEST_MEM_ATTR_CWO (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_TEST_MEM_ATTR_NCWO (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY_NC)
#define PLF_TEST_MEM_ATTR_NCSO (PMP_R | PMP_W | SCR_PMP_CTRL_MT_STRONG)

#define TEST_REGION_MRT_ENTRY \
	{PLF_DDR2_BASE, TEST_REGION_SIZE * CORES_NUM, PLF_MEM_ATTR, "Test", PLF_DEFAULT_MPORT}

/******************************************************************************
 *                              Types section
 */
enum caching_mode {
	CACHING_MODE_CWO_NCWO,
	CACHING_MODE_CWO_NCSO,

	CACHING_MODES_COUNT,
	CACHING_MODE_FIRST = CACHING_MODE_CWO_NCWO
};

/******************************************************************************
 *                              Static variables section
 */
static K_THREAD_STACK_ARRAY_DEFINE(tstack, CORES_NUM, STACK_SIZE);
static struct k_thread tthread[CORES_NUM];
static atomic_t synchronizers[CORES_NUM / BOUND_CORES_NUM];

static uint8_t patterns[CORES_NUM];
static uint32_t transactions_remaining[CORES_NUM / BOUND_CORES_NUM];

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
 * @brief Get synchronizer associated with particular CPU
 *
 * @param [in] core_id - CPU core ID
 *
 * @return synchronizer associated with particular CPU
 */
static inline atomic_t *get_synchronizer(uint8_t core_id)
{
	return &synchronizers[core_id / BOUND_CORES_NUM];
}

/**
 * @brief Reset remaining test transactions associated with particular CPU
 *
 * @param [in] core_id - CPU core ID
 */
static inline void reset_remained_transactions(uint8_t core_id)
{
	transactions_remaining[core_id / BOUND_CORES_NUM] = TEST_TRANSACTIONS_COUNT;
}

/**
 * @brief Get remaining test transactions associated with particular CPU
 *
 * @param [in] core_id - CPU core ID
 *
 * @return remaining test transactions associated with particular CPU
 */
static inline uint32_t get_remained_transactions(uint8_t core_id)
{
	return transactions_remaining[core_id / BOUND_CORES_NUM];
}

/**
 * @brief Decrement remaining test transactions associated with particular CPU
 *
 * @param [in] core_id - CPU core ID
 */
static inline uint32_t dec_remained_transactions(uint8_t core_id)
{
	return transactions_remaining[core_id / BOUND_CORES_NUM]--;
}

/**
 * @brief Get per-CPU pattern
 *
 * @param [in] core_id - CPU core ID
 *
 * @return per-CPU pattern
 */
static inline uint8_t get_pattern(uint8_t core_id)
{
	return patterns[core_id];
}

/**
 * @brief Set per-CPU pattern
 *
 * @param [in] core_id - CPU core ID
 * @param [in] pattern - pattern value
 */
static void set_pattern(uint8_t core_id, uint8_t pattern)
{
	patterns[core_id] = pattern;
}

/**
 * @brief Sets pattern value for the next check
 *
 * @param [in] core_id - CPU core ID
 */
static void set_next_pattern(uint8_t core_id)
{
	patterns[core_id] += BOUND_CORES_NUM;
}

/**
 * @brief Checks if CPU core id is odd
 *
 * @param [in] core_id - CPU core ID
 *
 * @return true, if CPU core id is odd; false - otherwise
 */
static inline bool is_odd_cpu(uint8_t core_id)
{
	return (core_id & 1) != 0;
}

/**
 * @brief Checks if CPU core id is even
 *
 * @param [in] core_id - CPU core ID
 *
 * @return true, if CPU core id is even; false - otherwise
 */
static inline bool is_even_cpu(uint8_t core_id)
{
	return (core_id & 1) == 0;
}

/**
 * @brief Test procedure: verify shared memory region and update
 * 		it with new pattern
 *
 * @param [in] core_id - CPU core id
 */
static void test_action(uint8_t core_id)
{
	uint32_t pattern = get_pattern(core_id);
	uint8_t *region = (uint8_t *)TEST_REGION_ADDR(core_id);

	/* Verify the pattern written by the other core */
	if (is_even_cpu(core_id)) {
		cache_data_invd_range(region, TEST_REGION_SIZE);
	}
	for (uint32_t offset = 0; offset < TEST_REGION_SIZE; offset++) {
		if (region[offset] != pattern) {
			printk("[%d] Data mismatch on CPU%d %d: have:%x expected:%x\n",
				TEST_TRANSACTIONS_COUNT - get_remained_transactions(core_id),
				core_id, offset, region[offset], pattern);
		}
	}

	/* Fill the region for the next core */
	memset(region, pattern + 1, TEST_REGION_SIZE);
	if (is_even_cpu(core_id)) {
		cache_data_flush_range(region, TEST_REGION_SIZE);
	}

	/* We expect new value to appear next time */
	set_next_pattern(core_id);
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
 * @brief Per-CPU caching mode configuration
 *
 * @param [in] core_id - CPU core id
 * @param [in] mode - caching mode
 */
static void configure_caching(uint8_t core_id, enum caching_mode mode)
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
			.base = TEST_REGION_ADDR(core_id),
			.size = TEST_REGION_SIZE,
			.name = "Test",
			.mrt_attr = PLF_DEFAULT_MPORT
		};
		const unsigned long attr_lookup[CACHING_MODES_COUNT][BOUND_CORES_NUM] = {
			[CACHING_MODE_CWO_NCWO] = {PLF_TEST_MEM_ATTR_CWO, PLF_TEST_MEM_ATTR_NCWO},
			[CACHING_MODE_CWO_NCSO] = {PLF_TEST_MEM_ATTR_CWO, PLF_TEST_MEM_ATTR_NCSO}
		};

		test_region_info.attr = attr_lookup[mode][is_even_cpu(core_id) ? 0 : 1];

		memcpy(&regions_map[TEST_REGION_MAP_INDEX],
			&test_region_info, sizeof(test_region_info));
		scr_pmp_reconfigure(regions_map);
	}
}

/**
 * @brief Per-CPU initialization
 *
 * @param [in] core_id - CPU core id
 * @param [in] mode - caching mode
 */
static void init_per_cpu(uint8_t core_id, enum caching_mode mode)
{
	configure_caching(core_id, mode);
	set_pattern(core_id, core_id % BOUND_CORES_NUM);

	if (is_even_cpu(core_id)) {
		memset((uint8_t *)TEST_REGION_ADDR(core_id), 0, TEST_REGION_SIZE);
		cache_data_flush_range((uint8_t *)TEST_REGION_ADDR(core_id), TEST_REGION_SIZE);
	}
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
	enum caching_mode mode = (uintptr_t)arg2;
	atomic_t *synchronizer = get_synchronizer(core_id);

	init_per_cpu(core_id, mode);

	while (42) {
		/* Wait for a turn of a particular core */
		while ((core_id % BOUND_CORES_NUM) != atomic_get(synchronizer)) {
			if (get_remained_transactions(core_id) == 0) {
				return;
			}
		}

		if (get_remained_transactions(core_id) == 0) {
			return;
		}

		test_action(core_id);
		dec_remained_transactions(core_id);

		if (is_odd_cpu(core_id)) {
			atomic_set(synchronizer, 0);
		} else {
			atomic_inc(synchronizer);
		}
	}
}

/**
 * @brief Test initialization
 *
 * @param [in] mode - caching mode
 */
static void test_init(enum caching_mode mode)
{
	memset(tthread, 0x00, sizeof(tthread));
	memset(tstack, 0x00, sizeof(tstack));

	for (uintptr_t core_id = 0; core_id < CORES_NUM; core_id++) {
		atomic_t *synchornizer = get_synchronizer(core_id);
		struct k_thread *thread = get_thread(core_id);

		atomic_set(synchornizer, 0);
		reset_remained_transactions(core_id);

		k_thread_create(thread, tstack[core_id], STACK_SIZE,
		   (k_thread_entry_t)test_thread,
		   (void *)core_id, (void *)mode, NULL,
		   K_PRIO_COOP(10), 0, K_FOREVER);

		k_thread_cpu_pin(thread, core_id);
	}
}

/**
 * @brief Test suite entry point
 */
int main(void)
{
	init_mrt();

	for (enum caching_mode mode = CACHING_MODE_FIRST; mode < CACHING_MODES_COUNT; mode++) {
		test_init(mode);

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

		printk("%d transactions completed on %d cores in mode %d!\n",
			TEST_TRANSACTIONS_COUNT, CORES_NUM, mode);
	}

	printk("All tests finished!\n");
	return 0;
}
