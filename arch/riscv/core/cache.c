/*
 * @file cache.c
 *
 * @brief RISC-V cache manipulation
 *
 * Copyright (c) 2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
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
void __weak dcache_invalidate(void *addr, long size)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
}

/**
 * @brief D-cache address range flush. Invalidation will also take place after the flush.
 *
 * @param [in] addr - range start address
 * @param [in] size - range size
 */
void __weak dcache_flush(void *addr, long size)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
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
	dcache_flush(addr, size);

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
	dcache_invalidate(addr, size);

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
	dcache_flush(addr, size);

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
