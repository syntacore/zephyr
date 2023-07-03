/**
 * Copyright (c) 2023 Syntacore
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * @file riscv_ipic.h
 * @author psk-sc
 * @brief Syntacore Integrated Programmable Interrupt Controller (IPIC) Interface
 *        for RISC-V processors
*/

#ifndef ZEPHYR_INCLUDE_DRIVERS_RISCV_IPIC_H_
#define ZEPHYR_INCLUDE_DRIVERS_RISCV_IPIC_H_

/**
 * @brief Enable raw external interrupt
 *
 * @param irq interrupt ID
 */
void riscv_ipic_irq_enable(uint32_t irq);

/**
 * @brief Disable raw external interrupt
 *
 * @param irq interrupt ID
 */
void riscv_ipic_irq_disable(uint32_t irq);

/**
 * @brief Check if an raw external interrupt is enabled
 *
 * @param irq interrupt ID
 * @return Returns true if interrupt is enabled, false otherwise
 */
int riscv_ipic_irq_is_enabled(uint32_t irq);

#endif /* ZEPHYR_INCLUDE_DRIVERS_RISCV_IPIC_H_ */