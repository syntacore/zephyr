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
 * @file ipi_iccm.c
 *
 * @brief Syntacore ICCM IPI SMP backend
 */

/******************************************************************************
 *                              Includes section
 */
#include <ipi.h>
#include <ksched.h>
#include <zephyr/kernel.h>
#include "iccm.h"

/******************************************************************************
 *                              Implementation section
 */

#ifdef CONFIG_SMP
#ifdef CONFIG_FPU_SHARING
/**
 * @brief Flush FPU context on specific CPU
 *
 * @param [in] cpu - particular cpu number
 */
void arch_flush_fpu_ipi(unsigned int cpu)
{
	ARG_UNUSED(cpu);
}

/**
 * @brief Perform architecture specific processing within spin loops
 *
 * This is invoked from busy loops with IRQs disabled such as the contended
 * spinlock loop. The default implementation is a weak function that calls
 * arch_nop(). Architectures may implement this function to perform extra
 * checks or power management tricks if needed.
 */
void arch_spin_relax(void)
{
}
#endif /* CONFIG_FPU_SHARING */

/**
 * Syntacore-specific SMP initialization
 */
int arch_smp_init(void)
{
	IRQ_CONNECT(RISCV_IRQ_MSOFT, 0, iccm_ipi_handler, NULL, 0);
	irq_enable(RISCV_IRQ_MSOFT);

	return 0;
}
#endif /* CONFIG_SMP */
