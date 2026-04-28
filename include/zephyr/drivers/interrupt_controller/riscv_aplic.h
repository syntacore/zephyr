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
 * @file riscv_aplic.h
 * 
 * @author mk-sc
 *
 * @brief Advanced Platform Level Interrupt Controller (APLIC) interface

 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_RISCV_APLIC_H_
#define ZEPHYR_INCLUDE_DRIVERS_RISCV_APLIC_H_

/**
 * @brief Enable interrupt
 *
 * @param irq interrupt ID
 */
void riscv_aplic_irq_enable(uint32_t irq);

/**
 * @brief Disable interrupt
 *
 * @param irq interrupt ID
 */
void riscv_aplic_irq_disable(uint32_t irq);

/**
 * @brief Check if an interrupt is enabled
 *
 * @param irq interrupt ID
 * @return Returns true if interrupt is enabled, false otherwise
 */
int riscv_aplic_irq_is_enabled(uint32_t irq);

/**
 * @brief Set interrupt priority
 *
 * @param irq interrupt ID
 * @param prio interrupt priority
 */
void riscv_aplic_set_priority(uint32_t irq, uint32_t prio);

/**
 * @brief Get active interrupt ID
 *
 * @return Returns the ID of an active interrupt
 */
int riscv_aplic_get_irq(void);

#endif /* ZEPHYR_INCLUDE_DRIVERS_RISCV_APLIC_H_ */
