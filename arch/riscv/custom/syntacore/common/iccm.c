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
 * @file iccm.c
 *
 * @brief Syntacore Inter-core communication module (ICCM) support
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <soc.h>
#include <ipi.h>

/******************************************************************************
 *                              Defines section
 */
#define SCR_ICCM_CSR_MBASE   0xbd8

#define SCR_ICCM_CSR_MADDR   (SCR_ICCM_CSR_MBASE + 0)
#define SCR_ICCM_CSR_MSTATUS (SCR_ICCM_CSR_MBASE + 1)
#define SCR_ICCM_CSR_MRDATA  (SCR_ICCM_CSR_MBASE + 2)
#define SCR_ICCM_CSR_MWDATA  (SCR_ICCM_CSR_MBASE + 3)

#define SCR_ICCM_MSG_SCHEDULE 1

/******************************************************************************
 *                              Implementation section
 */
#ifdef CONFIG_SMP

/**
 * @brief Select destination HART
 *
 * @param [in] dst_hart - destination HART id
 */
static inline void iccm_select_hart(uint8_t dst_hart)
{
	csr_write(SCR_ICCM_CSR_MADDR, dst_hart);
}

/**
 * @brief Write ICCM WDATA register.
 *		This will trigger message transmission to destination HART selected
 *
 * @param [in] data - data to send
 */
static inline void iccm_write_data(unsigned long data)
{
	csr_write(SCR_ICCM_CSR_MWDATA, data);
}

/**
 * @brief Read incoming msgbox contents: status (MSB), source slot num (low bits) and data.
 *		Incoming msgbox will be available for new messages
 *
 * @param [in] arg - device context
 *
 * @return Incoming msgbox contents
 */
static inline long iccm_get_data(void)
{
	long data = (long)csr_swap(SCR_ICCM_CSR_MRDATA, 1);

	return data;
}

/**
 * @brief Send ICCM message
 *
 * @param [in] dst_hart - destination HART
 * @param [in] data - message data
 */
static inline void iccm_send(uint8_t dst_hart, unsigned long data)
{
	iccm_select_hart(dst_hart);
	iccm_write_data(data);
}

/**
 * @brief RISC-V hook to deliver IPI to the set of CPU cores
 *		which will result in z_sched_ipi calls on other HARTs
 *
 * @param cpu_bitmap - mask of CPUs to deliver the IPI
 */
void arch_sched_directed_ipi(uint32_t cpu_bitmap)
{
	unsigned int key = arch_irq_lock();
	uint8_t current_hart_id = _current_cpu->id;
	uint8_t hart_id;
	uint8_t num_cpus = arch_num_cpus();

	for (hart_id = 0; hart_id < num_cpus; hart_id++) {
		if ((hart_id != current_hart_id) && ((cpu_bitmap & BIT(hart_id)) != 0)) {
			iccm_send(hart_id, SCR_ICCM_MSG_SCHEDULE);
		}
	}

	arch_irq_unlock(key);
}

/**
 * @brief RISC-V IPI interrupt handler
 */
void iccm_ipi_handler(const void *unused)
{
	iccm_get_data();
	z_sched_ipi();

	ARG_UNUSED(unused);
}

#endif /* CONFIG_SMP */
