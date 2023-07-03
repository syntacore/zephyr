#include "mwc.h"
#include "stress.h"
#include <stdlib.h>
#include <string.h>
#include <mmu.h>

#if IS_ENABLED(CONFIG_SCR_MMU)
unsigned long ddr_pte[ROUND_UP(PTE_COUNT(MMU_TRANSLATED_DDR_SIZE), 
				PTRS_PER_PTE)] __attribute__((aligned(PAGE_SIZE))) = {};
#endif /* CONFIG_SCR_MMU */

static volatile uint32_t masked_flags;
static volatile bool turn_on_mixing = MIXING;
static uint64_t disabled_flags;

static bool keep_stressing = false;

static shared_t test_info;

static volatile unsigned long long iterations = 0;
static const volatile unsigned long long max_iterations = MAX_ITERATIONS;

#define shim_builtin_prefetch __builtin_prefetch

#define CACHE_WRITE_MOD(flags)						\
	for (j = 0; j < test_mem_size; j++) {			\
		i += inc;						\
		i = (i >= test_mem_size) ? i - test_mem_size : i;	\
		k += 33;						\
		k = (k >= test_mem_size) ? k - test_mem_size : k;	\
									\
		if ((flags) & FLAGS_CACHE_PREFETCH) {			\
			shim_builtin_prefetch(&test_mem_ptr[i + 1], 1, 1);	\
		}							\
		if ((flags) & FLAGS_CACHE_CLDEMOTE) {			\
			SHIM_CLDEMOTE(&test_mem_ptr[i]);			\
		}							\
		if ((flags) & FLAGS_CACHE_CLFLUSHOPT) {			\
			SHIM_CLFLUSHOPT(&test_mem_ptr[i]);			\
		}							\
		test_mem_ptr[i] += test_mem_ptr[k] + r;			\
		if ((flags) & FLAGS_CACHE_CLWB) {			\
			SHIM_CLWB(&test_mem_ptr[i]);			\
		}							\
		if ((flags) & FLAGS_CACHE_CLFLUSH) {			\
			SHIM_CLFLUSH(&test_mem_ptr[i]);			\
		}							\
		if ((flags) & FLAGS_CACHE_FENCE) {			\
			shim_mfence();					\
		}									\
		if (!keep_stressing)				\
			break;						\
	}

#define CACHE_WRITE_USE_MOD(x)						\
static void stress_cache_write_mod_ ## x(			\
	const void *args,					\
	const uint64_t inc,						\
	const uint64_t r, 						\
	uint64_t *pi,							\
	uint64_t *pk,							\
	void *metrics)					\
{									\
	register uint64_t i = *pi, j, k = *pk;				\
	uint8_t *const test_mem_ptr = test_info.test_mem_ptr;			\
	const uint64_t test_mem_size = test_info.test_mem_size;	\
									\
	CACHE_WRITE_MOD(x); 					\
									\
									\
	*pi = i;							\
	*pk = k;							\
}									\

CACHE_WRITE_USE_MOD(0x00)
CACHE_WRITE_USE_MOD(0x01)
CACHE_WRITE_USE_MOD(0x02)
CACHE_WRITE_USE_MOD(0x03)
CACHE_WRITE_USE_MOD(0x04)
CACHE_WRITE_USE_MOD(0x05)
CACHE_WRITE_USE_MOD(0x06)
CACHE_WRITE_USE_MOD(0x07)
CACHE_WRITE_USE_MOD(0x08)
CACHE_WRITE_USE_MOD(0x09)
CACHE_WRITE_USE_MOD(0x0a)
CACHE_WRITE_USE_MOD(0x0b)
CACHE_WRITE_USE_MOD(0x0c)
CACHE_WRITE_USE_MOD(0x0d)
CACHE_WRITE_USE_MOD(0x0e)
CACHE_WRITE_USE_MOD(0x0f)

CACHE_WRITE_USE_MOD(0x10)
CACHE_WRITE_USE_MOD(0x11)
CACHE_WRITE_USE_MOD(0x12)
CACHE_WRITE_USE_MOD(0x13)
CACHE_WRITE_USE_MOD(0x14)
CACHE_WRITE_USE_MOD(0x15)
CACHE_WRITE_USE_MOD(0x16)
CACHE_WRITE_USE_MOD(0x17)
CACHE_WRITE_USE_MOD(0x18)
CACHE_WRITE_USE_MOD(0x19)
CACHE_WRITE_USE_MOD(0x1a)
CACHE_WRITE_USE_MOD(0x1b)
CACHE_WRITE_USE_MOD(0x1c)
CACHE_WRITE_USE_MOD(0x1d)
CACHE_WRITE_USE_MOD(0x1e)
CACHE_WRITE_USE_MOD(0x1f)

CACHE_WRITE_USE_MOD(0x20)
CACHE_WRITE_USE_MOD(0x21)
CACHE_WRITE_USE_MOD(0x22)
CACHE_WRITE_USE_MOD(0x23)
CACHE_WRITE_USE_MOD(0x24)
CACHE_WRITE_USE_MOD(0x25)
CACHE_WRITE_USE_MOD(0x26)
CACHE_WRITE_USE_MOD(0x27)
CACHE_WRITE_USE_MOD(0x28)
CACHE_WRITE_USE_MOD(0x29)
CACHE_WRITE_USE_MOD(0x2a)
CACHE_WRITE_USE_MOD(0x2b)
CACHE_WRITE_USE_MOD(0x2c)
CACHE_WRITE_USE_MOD(0x2d)
CACHE_WRITE_USE_MOD(0x2e)
CACHE_WRITE_USE_MOD(0x2f)

CACHE_WRITE_USE_MOD(0x30)
CACHE_WRITE_USE_MOD(0x31)
CACHE_WRITE_USE_MOD(0x32)
CACHE_WRITE_USE_MOD(0x33)
CACHE_WRITE_USE_MOD(0x34)
CACHE_WRITE_USE_MOD(0x35)
CACHE_WRITE_USE_MOD(0x36)
CACHE_WRITE_USE_MOD(0x37)
CACHE_WRITE_USE_MOD(0x38)
CACHE_WRITE_USE_MOD(0x39)
CACHE_WRITE_USE_MOD(0x3a)
CACHE_WRITE_USE_MOD(0x3b)
CACHE_WRITE_USE_MOD(0x3c)
CACHE_WRITE_USE_MOD(0x3d)
CACHE_WRITE_USE_MOD(0x3e)
CACHE_WRITE_USE_MOD(0x3f)

CACHE_WRITE_USE_MOD(0x40)
CACHE_WRITE_USE_MOD(0x41)
CACHE_WRITE_USE_MOD(0x42)
CACHE_WRITE_USE_MOD(0x43)
CACHE_WRITE_USE_MOD(0x44)
CACHE_WRITE_USE_MOD(0x45)
CACHE_WRITE_USE_MOD(0x46)
CACHE_WRITE_USE_MOD(0x47)
CACHE_WRITE_USE_MOD(0x48)
CACHE_WRITE_USE_MOD(0x49)
CACHE_WRITE_USE_MOD(0x4a)
CACHE_WRITE_USE_MOD(0x4b)
CACHE_WRITE_USE_MOD(0x4c)
CACHE_WRITE_USE_MOD(0x4d)
CACHE_WRITE_USE_MOD(0x4e)
CACHE_WRITE_USE_MOD(0x4f)

CACHE_WRITE_USE_MOD(0x50)
CACHE_WRITE_USE_MOD(0x51)
CACHE_WRITE_USE_MOD(0x52)
CACHE_WRITE_USE_MOD(0x53)
CACHE_WRITE_USE_MOD(0x54)
CACHE_WRITE_USE_MOD(0x55)
CACHE_WRITE_USE_MOD(0x56)
CACHE_WRITE_USE_MOD(0x57)
CACHE_WRITE_USE_MOD(0x58)
CACHE_WRITE_USE_MOD(0x59)
CACHE_WRITE_USE_MOD(0x5a)
CACHE_WRITE_USE_MOD(0x5b)
CACHE_WRITE_USE_MOD(0x5c)
CACHE_WRITE_USE_MOD(0x5d)
CACHE_WRITE_USE_MOD(0x5e)
CACHE_WRITE_USE_MOD(0x5f)

CACHE_WRITE_USE_MOD(0x60)
CACHE_WRITE_USE_MOD(0x61)
CACHE_WRITE_USE_MOD(0x62)
CACHE_WRITE_USE_MOD(0x63)
CACHE_WRITE_USE_MOD(0x64)
CACHE_WRITE_USE_MOD(0x65)
CACHE_WRITE_USE_MOD(0x66)
CACHE_WRITE_USE_MOD(0x67)
CACHE_WRITE_USE_MOD(0x68)
CACHE_WRITE_USE_MOD(0x69)
CACHE_WRITE_USE_MOD(0x6a)
CACHE_WRITE_USE_MOD(0x6b)
CACHE_WRITE_USE_MOD(0x6c)
CACHE_WRITE_USE_MOD(0x6d)
CACHE_WRITE_USE_MOD(0x6e)
CACHE_WRITE_USE_MOD(0x6f)

CACHE_WRITE_USE_MOD(0x70)
CACHE_WRITE_USE_MOD(0x71)
CACHE_WRITE_USE_MOD(0x72)
CACHE_WRITE_USE_MOD(0x73)
CACHE_WRITE_USE_MOD(0x74)
CACHE_WRITE_USE_MOD(0x75)
CACHE_WRITE_USE_MOD(0x76)
CACHE_WRITE_USE_MOD(0x77)
CACHE_WRITE_USE_MOD(0x78)
CACHE_WRITE_USE_MOD(0x79)
CACHE_WRITE_USE_MOD(0x7a)
CACHE_WRITE_USE_MOD(0x7b)
CACHE_WRITE_USE_MOD(0x7c)
CACHE_WRITE_USE_MOD(0x7d)
CACHE_WRITE_USE_MOD(0x7e)
CACHE_WRITE_USE_MOD(0x7f)

static const cache_mixed_ops_func_t cache_mixed_ops_funcs[] = {
	stress_cache_write_mod_0x00,
	stress_cache_write_mod_0x01,
	stress_cache_write_mod_0x02,
	stress_cache_write_mod_0x03,
	stress_cache_write_mod_0x04,
	stress_cache_write_mod_0x05,
	stress_cache_write_mod_0x06,
	stress_cache_write_mod_0x07,
	stress_cache_write_mod_0x08,
	stress_cache_write_mod_0x09,
	stress_cache_write_mod_0x0a,
	stress_cache_write_mod_0x0b,
	stress_cache_write_mod_0x0c,
	stress_cache_write_mod_0x0d,
	stress_cache_write_mod_0x0e,
	stress_cache_write_mod_0x0f,

	stress_cache_write_mod_0x10,
	stress_cache_write_mod_0x11,
	stress_cache_write_mod_0x12,
	stress_cache_write_mod_0x13,
	stress_cache_write_mod_0x14,
	stress_cache_write_mod_0x15,
	stress_cache_write_mod_0x16,
	stress_cache_write_mod_0x17,
	stress_cache_write_mod_0x18,
	stress_cache_write_mod_0x19,
	stress_cache_write_mod_0x1a,
	stress_cache_write_mod_0x1b,
	stress_cache_write_mod_0x1c,
	stress_cache_write_mod_0x1d,
	stress_cache_write_mod_0x1e,
	stress_cache_write_mod_0x1f,

	stress_cache_write_mod_0x20,
	stress_cache_write_mod_0x21,
	stress_cache_write_mod_0x22,
	stress_cache_write_mod_0x23,
	stress_cache_write_mod_0x24,
	stress_cache_write_mod_0x25,
	stress_cache_write_mod_0x26,
	stress_cache_write_mod_0x27,
	stress_cache_write_mod_0x28,
	stress_cache_write_mod_0x29,
	stress_cache_write_mod_0x2a,
	stress_cache_write_mod_0x2b,
	stress_cache_write_mod_0x2c,
	stress_cache_write_mod_0x2d,
	stress_cache_write_mod_0x2e,
	stress_cache_write_mod_0x2f,

	stress_cache_write_mod_0x30,
	stress_cache_write_mod_0x31,
	stress_cache_write_mod_0x32,
	stress_cache_write_mod_0x33,
	stress_cache_write_mod_0x34,
	stress_cache_write_mod_0x35,
	stress_cache_write_mod_0x36,
	stress_cache_write_mod_0x37,
	stress_cache_write_mod_0x38,
	stress_cache_write_mod_0x39,
	stress_cache_write_mod_0x3a,
	stress_cache_write_mod_0x3b,
	stress_cache_write_mod_0x3c,
	stress_cache_write_mod_0x3d,
	stress_cache_write_mod_0x3e,
	stress_cache_write_mod_0x3f,

	stress_cache_write_mod_0x40,
	stress_cache_write_mod_0x41,
	stress_cache_write_mod_0x42,
	stress_cache_write_mod_0x43,
	stress_cache_write_mod_0x44,
	stress_cache_write_mod_0x45,
	stress_cache_write_mod_0x46,
	stress_cache_write_mod_0x47,
	stress_cache_write_mod_0x48,
	stress_cache_write_mod_0x49,
	stress_cache_write_mod_0x4a,
	stress_cache_write_mod_0x4b,
	stress_cache_write_mod_0x4c,
	stress_cache_write_mod_0x4d,
	stress_cache_write_mod_0x4e,
	stress_cache_write_mod_0x4f,

	stress_cache_write_mod_0x50,
	stress_cache_write_mod_0x51,
	stress_cache_write_mod_0x52,
	stress_cache_write_mod_0x53,
	stress_cache_write_mod_0x54,
	stress_cache_write_mod_0x55,
	stress_cache_write_mod_0x56,
	stress_cache_write_mod_0x57,
	stress_cache_write_mod_0x58,
	stress_cache_write_mod_0x59,
	stress_cache_write_mod_0x5a,
	stress_cache_write_mod_0x5b,
	stress_cache_write_mod_0x5c,
	stress_cache_write_mod_0x5d,
	stress_cache_write_mod_0x5e,
	stress_cache_write_mod_0x5f,

	stress_cache_write_mod_0x60,
	stress_cache_write_mod_0x61,
	stress_cache_write_mod_0x62,
	stress_cache_write_mod_0x63,
	stress_cache_write_mod_0x64,
	stress_cache_write_mod_0x65,
	stress_cache_write_mod_0x66,
	stress_cache_write_mod_0x67,
	stress_cache_write_mod_0x68,
	stress_cache_write_mod_0x69,
	stress_cache_write_mod_0x6a,
	stress_cache_write_mod_0x6b,
	stress_cache_write_mod_0x6c,
	stress_cache_write_mod_0x6d,
	stress_cache_write_mod_0x6e,
	stress_cache_write_mod_0x6f,

	stress_cache_write_mod_0x70,
	stress_cache_write_mod_0x71,
	stress_cache_write_mod_0x72,
	stress_cache_write_mod_0x73,
	stress_cache_write_mod_0x74,
	stress_cache_write_mod_0x75,
	stress_cache_write_mod_0x76,
	stress_cache_write_mod_0x77,
	stress_cache_write_mod_0x78,
	stress_cache_write_mod_0x79,
	stress_cache_write_mod_0x7a,
	stress_cache_write_mod_0x7b,
	stress_cache_write_mod_0x7c,
	stress_cache_write_mod_0x7d,
	stress_cache_write_mod_0x7e,
	stress_cache_write_mod_0x7f,
};

void stress_cache_read(
	const void *args,
	uint8_t *test_mem_ptr,
	const uint64_t test_mem_size,
	const uint64_t inc,
	uint64_t *i_ptr,
	uint64_t *k_ptr,
	void *metrics_read)
{
	register uint64_t i = *i_ptr;
	register uint64_t j;
	register uint64_t k = *k_ptr;
	uint32_t total = 0;

	for (j = 0; j < test_mem_size; j++) {
		i += inc;
		i = (i >= test_mem_size) ? i - test_mem_size : i;
		k += 33;
		k = (k >= test_mem_size) ? k - test_mem_size : k;
		total += test_mem_ptr[i] + test_mem_ptr[k];
		if (!keep_stressing)
			break;
	}

	*i_ptr = i;
	*k_ptr = k;
}

void stress_cache_write(
	const void *args,
	uint8_t *test_mem_ptr,
	const uint64_t test_mem_size,
	const uint64_t inc,
	uint64_t *i_ptr,
	uint64_t *k_ptr,
	void *metrics_write)
{
	register uint64_t i = *i_ptr;
	register uint64_t j;
	register uint64_t k = *k_ptr;
	
	for (j = 0; j < test_mem_size; j++) {
		register uint8_t v = j & 0xff;
		i += inc;
		i = (i >= test_mem_size) ? i - test_mem_size : i;
		k += 33;
		k = (k >= test_mem_size) ? k - test_mem_size : k;
		test_mem_ptr[i] = v;
		test_mem_ptr[k] = v;
		if (!keep_stressing)
			break;
	}

	*i_ptr = i;
	*k_ptr = k;
}

/**
 *  @brief stress cache by psuedo-random memory read/writes
 */
void stress_cache(void) 
{
/*  
 * In the current configuration, functions
 * with MMU cannot write to other devices
 */
#if !IS_ENABLED(CONFIG_SCR_MMU)
	printk("routine start\n");
#endif

	uint32_t cache_flags = CACHE_FLAGS;
	uint8_t *const test_mem_ptr = test_info.test_mem_ptr;
	const uint64_t test_mem_size = test_info.test_mem_size;
	uint64_t i = stress_mwc64modn(test_mem_size);
	uint64_t k = i + (test_mem_size >> 1);
	uint64_t r = 0;
	uint64_t inc = (test_mem_size >> 2) + 1;

	disabled_flags = 0; 

	masked_flags = cache_flags & FLAGS_CACHE_MASK;

	do {

		iterations++;
		if (iterations > max_iterations) {
			keep_stressing = false;
			break;
		}
		uint32_t flags;
		switch (r) {
		case STRESS_CACHE_MIXED_OPS:
			flags = masked_flags;
			if (turn_on_mixing) {
				flags = (stress_mwc32() & FLAGS_CACHE_MASK) & masked_flags;
			}
			cache_mixed_ops_funcs[flags](NULL, inc, r, &i, &k, NULL);
			break;
		case STRESS_CACHE_READ:
			stress_cache_read(NULL, test_mem_ptr, test_mem_size, inc, &i, &k, NULL);
			break;
		case STRESS_CACHE_WRITE:
			stress_cache_write(NULL, test_mem_ptr, test_mem_size, inc, &i, &k, NULL);
			break;
		}
		r++;
		if (r >= STRESS_CACHE_MAX)
			r = 0;

		(void)shim_cacheflush((char *)stress_cache, 8192, SHIM_ICACHE);
		(void)shim_cacheflush((char *)test_mem_ptr, (int)test_mem_size, SHIM_DCACHE);

		cache_data_flush_range((void *)stress_cache, 64);

		/* Move forward a bit */
		i += inc;
		i = (i >= test_mem_size) ? i - test_mem_size : i;

	} while (keep_stressing);

#if !IS_ENABLED(CONFIG_SCR_MMU)
	printk("routine finished\n");
#endif
}

#if IS_ENABLED(CONFIG_SCR_MMU)
/**
 *	@brief Wrapper for MMU and S-mode
 */
void stress_wrapper()
{
	mmu_run_in_s_mode(stress_cache);
}
#endif

static K_THREAD_STACK_ARRAY_DEFINE(tstack, THREADS_NUM, STACK_SIZE);
static k_tid_t tinfo[THREADS_NUM];
static struct k_thread tthread[THREADS_NUM];

static void threads_init(void)
{
	memset(tthread, 0x00, sizeof(tthread));
	memset(tinfo, 0x00, sizeof(tinfo));
	memset(tstack, 0x00, sizeof(tstack));
}

static uint8_t test_buffer[TEST_MEM_SIZE];

#if IS_ENABLED(CONFIG_SCR_MMU)
#define THREAD_ROUTINE stress_wrapper
#else
#define THREAD_ROUTINE stress_cache
#endif

void main()
{
	test_info.test_mem_ptr = test_buffer;
	test_info.test_mem_size = TEST_MEM_SIZE;

	printk("Mem_cache_ptr - %p, size - %d\n", test_info.test_mem_ptr, TEST_MEM_SIZE);

	threads_init();

	keep_stressing = true;

	uint64_t start_time, stop_time, cycles_spent, nanoseconds_spent;
	start_time = k_cycle_get_64();

	for (int i = 0; i < THREADS_NUM; i++) {

		tinfo[i] = k_thread_create(&tthread[i], tstack[i], STACK_SIZE,
					(k_thread_entry_t) THREAD_ROUTINE,
					NULL, NULL, NULL,
					K_PRIO_PREEMPT(PRIORITY), 0, K_NO_WAIT);
	}

	printk("All thread created\n");

	for (int i = 0; i < THREADS_NUM; i++) {

		k_thread_join(tinfo[i], K_FOREVER);
	}

	stop_time = k_cycle_get_64();

	cycles_spent = stop_time - start_time;
	nanoseconds_spent = (uint64_t)k_cyc_to_ns_floor64(cycles_spent);

	printk("All %lld iterations on all %d threads executed in %lld msec\n", 
					iterations, THREADS_NUM, nanoseconds_spent / 1000 / 1000);

	printk("TEST PASSED\n");
}
