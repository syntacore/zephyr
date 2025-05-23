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
 * @file halt.c
 *
 * @brief Syntacore custom system halt handler
 *
 * @author amr-sc
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
