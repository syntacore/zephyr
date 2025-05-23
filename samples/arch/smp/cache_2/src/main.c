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
 * @brief Syntacore Cache flush/invalidate Test #2
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
#include <cache.h>

/******************************************************************************
 *                              Defines section
 */
#define STACK_SIZE	(8 * 1024)

#define CORES_NUM	CONFIG_MP_MAX_NUM_CPUS

#define TEST_REGION_SIZE DT_PROP(DT_NODELABEL(l2), d_cache_size)
#define TEST_REGION_ADDR(cpu_id) (PLF_DDR2_BASE + (cpu_id) * TEST_REGION_SIZE)
#define TEST_TRANSACTIONS_COUNT 1000
#define TEST_SEQUENCE_FRAGMENT_SIZE 256

#define PLF_TEST_MEM_ATTR_CACHED (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_TEST_MEM_ATTR_UNCACHED (PMP_R | PMP_W | SCR_PMP_CTRL_MT_WEAKLY_NC)

#define TEST_REGION_MRT_ENTRY \
	{PLF_DDR2_BASE, TEST_REGION_SIZE * CORES_NUM, PLF_MEM_ATTR, "Test", PLF_DEFAULT_MPORT}

/* Test region index is the first in the memory regions map */
#define TEST_REGION_MAP_INDEX (0)

#define PATTERN_5A 0x5A5A5A5A5A5A5A5AULL
#define PATTERN_F0 0xF0F0F0F0F0F0F0F0ULL

#define FENCE_TEST_STEP_SIZE DT_PROP(DT_NODELABEL(l2), d_cache_line_size)
#define FENCE_TEST_STEPS_COUNT 32

enum caching_mode {
	CACHING_MODE_ON,
	CACHING_MODE_OFF,
};

typedef void (*test_t)(uint8_t core_id);

/******************************************************************************
 *                              Static variables section
 */
static K_THREAD_STACK_ARRAY_DEFINE(tstack, CORES_NUM, STACK_SIZE);
static struct k_thread tthread[CORES_NUM];
static uint32_t transactions_remaining = TEST_TRANSACTIONS_COUNT;
static atomic_t start_sync, completion_sync;
static K_MUTEX_DEFINE(lock);

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
static inline uint64_t get_pattern(void)
{
	if (transactions_remaining & 1) {
		return PATTERN_5A;
	} else {
		return PATTERN_F0;
	}
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
	for (uint64_t *ptr = start;	(uintptr_t)ptr < (uintptr_t)start + size; ptr++) {
		*ptr = pattern;
	}
}

/**
 * @brief Verifies pattern in memory
 *
 * @param [in] start - start address of memory
 * @param [in] pattern - expected pattern
 * @param [in] size - memory size
 * @param [in] core_id - CPU core id
 */
static void verify_pattern(uint64_t *start, uint64_t pattern, size_t size, uint8_t core_id)
{
	for (uint64_t *ptr = start;	(uintptr_t)ptr < (uintptr_t)start + size; ptr++) {
		if (*ptr != pattern) {
			printf("ERROR core%d:%p=%llx expected:%llx\n",
					core_id, ptr, *ptr, pattern);
			return;
		}
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
 * @brief Test procedure: verify shared memory region and update
 * 		it with new pattern
 *
 * @param [in] core_id - CPU core id
 */
static void main_test(uint8_t core_id)
{
	uint64_t pattern = get_pattern();
	uint64_t *ptr = (uint64_t *)TEST_REGION_ADDR(core_id);

	while ((uintptr_t)ptr < TEST_REGION_ADDR(core_id) + TEST_REGION_SIZE)
	{
		/* Fill the fragment with pattern */
		fill_pattern(ptr, pattern, TEST_SEQUENCE_FRAGMENT_SIZE);
		/* Flush the fragment */
		cache_data_flush_range(ptr, TEST_SEQUENCE_FRAGMENT_SIZE);
		/* Disable caching */
		configure_caching(core_id, CACHING_MODE_OFF);
		/* Verify pattern in DDR */
		verify_pattern(ptr, pattern, TEST_SEQUENCE_FRAGMENT_SIZE, core_id);
		/* Proceed as cached */
		configure_caching(core_id, CACHING_MODE_ON);

		ptr = (uint64_t *)((uintptr_t)ptr + TEST_SEQUENCE_FRAGMENT_SIZE);
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
 * @brief Runs test synchronously for all the cores
 *
 * @param [in] core_id - CPU core id
 * @param [in] test - test function
 */
static void do_test(uint8_t core_id, test_t test)
{
	/* Synchronize all the cores */
	while (atomic_get(&start_sync) == 0)
		;

	if (!transactions_remaining)
		return;

	atomic_dec(&start_sync);
	while (atomic_get(&start_sync) != 0)
		;

	test(core_id);

	atomic_inc(&completion_sync);

	if (core_id == 0) {
		/* Wait till all the cores complete the transaction */
		while (atomic_get(&completion_sync) != CORES_NUM)
			;
		transactions_remaining--;

		/* Unblock next transaction */
		atomic_set(&completion_sync, 0);
		atomic_set(&start_sync, CORES_NUM);
	}
}

/**
 * @brief Gets pattern for the fence test
 *
 * @param [in] core_id - CPU core id
 * @param [in] step - current step number
 *
 * @return pattern for the fence test
 */
static inline uint64_t get_fence_test_pattern(uint32_t step)
{
	return 0xBABADEDA00000000 + step;
}

/**
 * @brief Gets slot for the fence test
 *
 * @param [in] core_id - CPU core id
 * @param [in] step - current step number
 *
 * @return slot for the fence test
 */
static inline uint64_t *get_fence_test_slot(uint8_t core_id, uint32_t step)
{
	return (uint64_t *)(TEST_REGION_ADDR(core_id) + (step * FENCE_TEST_STEP_SIZE));
}

/**
 * @brief Fence test for #31037
 *		Flush wasn't executed properly if fence+flush was done fast enough
 *		after the writes to memory.
 *
 * @param [in] core_id - CPU core id
 */
static void fence_test(uint8_t core_id)
{
	/* Start with caching on */
	configure_caching(core_id, CACHING_MODE_ON);

	/* Fill the caches with pattern */
	for (int step = 0; step < FENCE_TEST_STEPS_COUNT; step++) {
		uint64_t *ptr = get_fence_test_slot(core_id, step);

		*ptr = get_fence_test_pattern(step);
	};

	/* Fence and L3 flush should come ASAP after writing */
	fence();
	scr_cache_l3_flush();

	/* Disable caching */
	configure_caching(core_id, CACHING_MODE_OFF);

	/* Verify pattern in DDR */
	for (int step = 0; step < FENCE_TEST_STEPS_COUNT; step++) {
		uint64_t *ptr = get_fence_test_slot(core_id, step);
		uint64_t pattern = get_fence_test_pattern(step);

		if (*ptr != pattern) {
			printf("Fence test error core%d:%p=%llx expected:%llx\n",
				core_id, ptr, *ptr, pattern);
		}
	};

	/* Enable back caching */
	configure_caching(core_id, CACHING_MODE_ON);
}

/**
 * @brief Cache eviction sanity test
 *		We fill the entire L2 and check that no unexpected eviction occurs
 *
 * @param [in] core_id - CPU core id
 */
static void eviction_test(uint8_t core_id)
{
	k_mutex_lock(&lock, K_FOREVER);
	printk("Eviction test starts for core%d\n", core_id);
	/* Clean-up the caches before the test */
	cache_data_invd_range((uint8_t *)TEST_REGION_ADDR(core_id), TEST_REGION_SIZE);
	/* Start as uncached */
	configure_caching(core_id, CACHING_MODE_OFF);
	/* Fill the region with pattern */
	fill_pattern((uint64_t *)TEST_REGION_ADDR(core_id), PATTERN_5A, TEST_REGION_SIZE);
	/* Enable caching */
	configure_caching(core_id, CACHING_MODE_ON);
	/* Fill with different pattern */
	fill_pattern((uint64_t *)TEST_REGION_ADDR(core_id), PATTERN_F0, TEST_REGION_SIZE);
	/* Ensure the pattern remained in caches and no unexpected eviction appeared */
	cache_data_invd_range((uint8_t *)TEST_REGION_ADDR(core_id), TEST_REGION_SIZE);
	verify_pattern((uint64_t *)TEST_REGION_ADDR(core_id), PATTERN_5A, TEST_REGION_SIZE, core_id);
	/* Clean-up the caches after the test */
	cache_data_invd_range((uint8_t *)TEST_REGION_ADDR(core_id), TEST_REGION_SIZE);

	printk("Eviction test done for core%d\n", core_id);
	k_mutex_unlock(&lock);
}

/**
 * @brief Per-CPU initialization
 *
 * @param [in] core_id - CPU core id
 */
static void init_per_cpu(uint8_t core_id)
{
	configure_caching(core_id, CACHING_MODE_ON);
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

	while (transactions_remaining) {
		/* Run cache eviction sanity test */
		do_test(core_id, eviction_test);
		/* Run fence test */
		do_test(core_id, fence_test);
		/* Run main test */
		do_test(core_id, main_test);
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
	atomic_set(&completion_sync, 0);
	atomic_set(&start_sync, CORES_NUM);

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
