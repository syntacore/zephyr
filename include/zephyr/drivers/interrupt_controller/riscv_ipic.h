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
 * @file riscv_ipic.h
 * 
 * @author psk-sc
 *
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
