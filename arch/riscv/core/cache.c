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
 * @brief RISC-V cache manipulation
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/cache.h>

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief D-cache address range invalidate
 *
 * @param [in] addr - range start address
 * @param [in] size - range size
 */
void __weak dcache_invalidate(uintptr_t addr, long size)
{
#if IS_ENABLED(CONFIG_RISCV_ISA_EXT_ZICBOM)
	fence();

	if (size) {
		do {
			__asm__ volatile("cbo.inval (%0)" ::"r"(addr) : "memory");
			addr += CONFIG_DCACHE_LINE_SIZE;
			size -= CONFIG_DCACHE_LINE_SIZE;
		} while (size > 0);
	}
#else
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
#endif /* IS_ENABLED(CONFIG_RISCV_ISA_EXT_ZICBOM) */
}

/**
 * @brief D-cache address range flush. Invalidation will also take place after the flush.
 *
 * @param [in] addr - range start address
 * @param [in] size - range size
 */
void __weak dcache_flush(uintptr_t addr, long size)
{
#if IS_ENABLED(CONFIG_RISCV_ISA_EXT_ZICBOM)
	if (size) {
		/* Flush the cache for the requested range */
		do {
			__asm__ volatile("cbo.flush (%0)" ::"r"(addr) : "memory");
			addr += CONFIG_DCACHE_LINE_SIZE;
			size -= CONFIG_DCACHE_LINE_SIZE;
		} while (size > 0);
	}

	fence();
#else
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
#endif /* IS_ENABLED(CONFIG_RISCV_ISA_EXT_ZICBOM) */
}

/**
 * @brief Flush an address range in the D-cache
 *
 * Flush the specified address range of the data cache.
 *
 * @param addr Starting address to flush.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_flush_range(void *addr, size_t size)
{
	dcache_flush((uintptr_t)addr, size);

	return 0;
}

/**
 * @brief Invalidate an address range in the D-cache
 *
 * Invalidate the specified address range of the data cache.
 *
 * @param addr Starting address to invalidate.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_invd_range(void *addr, size_t size)
{
	dcache_invalidate((uintptr_t)addr, size);

	return 0;
}

/**
 * @brief Flush and Invalidate an address range in the D-cache
 *
 * Flush and Invalidate the specified address range of the data cache.
 *
 * @param addr Starting address to flush and invalidate.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_flush_and_invd_range(void *addr, size_t size)
{
	dcache_flush((uintptr_t)addr, size);

	return 0;
}

/**
 * @brief Enable the D-cache
 *
 * Enable the data cache
 *
 */
void arch_dcache_enable(void)
{
	/* Not supported yet */
}

/**
 * @brief Disable the D-cache
 *
 * Disable the data cache
 *
 */
void arch_dcache_disable(void)
{
	/* Not supported yet */
}

/**
 * @brief Flush the D-cache
 *
 * Flush the whole data cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_flush_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Invalidate the D-cache
 *
 * Invalidate the whole data cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_invd_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Flush and Invalidate the D-cache
 *
 * Flush and Invalidate the whole data cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_dcache_flush_and_invd_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Enable the I-cache
 *
 * Enable the instruction cache
 *
 */
void arch_icache_enable(void)
{
	/* Not supported yet */
}

/**
 * @brief Disable the I-cache
 *
 * Disable the instruction cache
 *
 */
void arch_icache_disable(void)
{
	/* Not supported yet */
}

/**
 * @brief Flush the I-cache
 *
 * Flush the whole instruction cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_flush_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Invalidate the I-cache
 *
 * Invalidate the whole instruction cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_invd_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Flush and Invalidate the I-cache
 *
 * Flush and Invalidate the whole instruction cache.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_flush_and_invd_all(void)
{
	return -ENOTSUP;
}

/**
 * @brief Flush an address range in the I-cache
 *
 * Flush the specified address range of the instruction cache.
 *
 * @param addr Starting address to flush.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_flush_range(void *addr, size_t size)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(size);

	return -ENOTSUP;
}

/**
 * @brief Invalidate an address range in the I-cache
 *
 * Invalidate the specified address range of the instruction cache.
 *
 * @param addr Starting address to invalidate.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_invd_range(void *addr, size_t size)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(size);

	return -ENOTSUP;
}

/**
 * @brief Flush and Invalidate an address range in the I-cache
 *
 * Flush and Invalidate the specified address range of the instruction cache.
 *
 * @param addr Starting address to flush and invalidate.
 * @param size Range size.
 *
 * @retval 0 If succeeded.
 * @retval -ENOTSUP If not supported.
 * @retval -errno Negative errno for other failures.
 */
int arch_icache_flush_and_invd_range(void *addr, size_t size)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(size);

	return -ENOTSUP;
}
