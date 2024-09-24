
#include <zephyr/arch/riscv/arch_inlines.h>
#include <zephyr/kernel.h>
#include <zephyr/cache.h>
#include <stdint.h>

#ifndef	_STRESS_H
#define	_STRESS_H
#endif

#define FLAGS_CACHE_PREFETCH	(0x0001U)
#define FLAGS_CACHE_CLFLUSH	(0x0002U)
#define FLAGS_CACHE_FENCE	(0x0004U)
#define FLAGS_CACHE_SFENCE	(0x0008U)
#define FLAGS_CACHE_CLFLUSHOPT	(0x0010U)
#define FLAGS_CACHE_CLDEMOTE	(0x0020U)
#define FLAGS_CACHE_CLWB	(0x0040U)
#define FLAGS_CACHE_NOAFF	(0x8000U)

#define STRESS_CACHE_MIXED_OPS	(0)
#define STRESS_CACHE_READ	(1)
#define STRESS_CACHE_WRITE	(2)
#define STRESS_CACHE_MAX	(3)

typedef void (*cache_mixed_ops_func_t)(const void *args,
	uint64_t inc, const uint64_t r,
	uint64_t *pi, uint64_t *pk,
	void *metrics);

#define FLAGS_CACHE_MASK	(FLAGS_CACHE_PREFETCH |		\
				 FLAGS_CACHE_CLFLUSH |		\
				 FLAGS_CACHE_FENCE |		\
				 FLAGS_CACHE_SFENCE |		\
				 FLAGS_CACHE_CLFLUSHOPT |	\
				 FLAGS_CACHE_CLDEMOTE |		\
				 FLAGS_CACHE_CLWB)

/* * * * * PARAMETRS * * * * */

#define STACK_SIZE 8192 
#define THREADS_NUM 1

/*
 * Consider if you want TIME_SLICING
 * PRIORITY < CONFIG_TIMESLICE_PRIORITY - NO Time Slicing
 * And if you want time slicing: CONFIG_TIMESLICING=y in prj.conf
 */
#define PRIORITY 10

/*
 * This is flags will be used, max - FLAGS_CACHE_MASK
 */
#define CACHE_FLAGS 0 	

/*
 * True  - playback with different flags
 * False - playback only with flag CACHE_FLAGS
 */
#define MIXING false

#define TEST_MEM_SIZE (DT_PROP(DT_NODELABEL(l2), d_cache_size) * 2)
#define MAX_ITERATIONS (500 * THREADS_NUM)

/* * *  END OF PARAMETRS  * * */

#define SHIM_ICACHE	(1 << 0)
#define SHIM_DCACHE	(1 << 1)

typedef struct {
	uint32_t flag;
	k_tid_t *tid;
} setting_t;

typedef struct {
	uint8_t	*test_mem_ptr;		/*!< Shared memory cache */
	uint64_t test_mem_size;		/*!< In bytes */
} shared_t;

int shim_cacheflush(char *addr, int nbytes, int cache);

/*
 *  @brief serializing memory fence
 */
static inline void ALWAYS_INLINE shim_mfence(void)
{
	fence();
	__sync_synchronize();
}

#if defined(HAVE_ASM_X86_CLFLUSH)
#define SHIM_CLFLUSH(p)		stress_asm_x86_clflush(p)
#else
#define SHIM_CLFLUSH(p)
#endif

#if defined(HAVE_ASM_X86_CLFLUSHOPT)
#define SHIM_CLFLUSHOPT(p)	stress_asm_x86_clflushopt(p)
#else
#define SHIM_CLFLUSHOPT(p)
#endif

#if defined(HAVE_ASM_X86_CLDEMOTE)
#define SHIM_CLDEMOTE(p)	stress_asm_x86_cldemote(p)
#else
#define SHIM_CLDEMOTE(p)
#endif

#if defined(HAVE_ASM_X86_CLWB)
#define SHIM_CLWB(p)		stress_asm_x86_clwb(p)
#else
#define SHIM_CLWB(p)
#endif