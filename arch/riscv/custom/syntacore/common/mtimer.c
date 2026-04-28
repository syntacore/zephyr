/*
 * Copyright (C) 2025, Syntacore Ltd.
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
 * @file mtimer.c
 *
 * @brief Syntacore fixup for standard RISC-V MTIMER
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/devicetree.h>

/******************************************************************************
 *                              Defines section
 */
#define MTIMER_NODE DT_NODELABEL(mtimer)
#define MTIMEDIV_REG DT_REG_ADDR_BY_NAME_U64(MTIMER_NODE, mtimediv)

/******************************************************************************
 *                              Implementation section
 */

/*
 * @brief Sets up MTIMER divider
 */
static void set_divider(void)
{
	*(volatile uint32_t *)MTIMEDIV_REG =
		CONFIG_RISCV_MACHINE_TIMER_SYSTEM_CLOCK_DIVIDER;
}

/*
 * @brief Fixes-up MTIMER registers which couldn't be initialized
 *		with standard driver API
 */
void mtimer_early_init(void)
{
	set_divider();
}
