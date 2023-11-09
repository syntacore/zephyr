/*
 * @file halt.c
 *
 * @brief Syntacore custom system halt handler
 *
 * @author amr-sc
 *
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>

#if IS_ENABLED(CONFIG_SCR_SYSTEM_HALT)

/******************************************************************************
 *                              Defines section
 */
#if __riscv_xlen == 32
#define TEST_ADDR      (0xf0080000)
#define TEST_ADDR_BASE ((uint32_t*)(TEST_ADDR))
#else
#define TEST_ADDR      (0xffffffffff008000)
#define TEST_ADDR_BASE ((uint64_t*)(TEST_ADDR))
#endif /* __riscv_xlen == 32 */

/******************************************************************************
 *                              Implementation section
 */
__weak void sc_exit(unsigned int reason)
{
#if __riscv_xlen == 32
    *TEST_ADDR_BASE = (reason) ? (0x3333 | ((uint32_t)reason << 16)) : 0x5555;
#else
    *TEST_ADDR_BASE = (reason) ? (0x3333 | ((uint64_t)reason << 16)) : 0x5555;
#endif /* __riscv_xlen == 32 */
}

/**
 * @brief Syntacore custom system halt handler
 *
 * @param [in] reason - halt reason
 */
void arch_system_halt(unsigned int reason)
{
	sc_exit(CONFIG_SCR_SYSTEM_HALT_REASON_OFFSET + reason);
}

#endif /* CONFIG_SCR_SYSTEM_HALT */
