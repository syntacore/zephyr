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

#ifndef ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_IMSIC_H_
#define ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_IMSIC_H_

#include <zephyr/kernel.h>

#define AIA_REG64_POW2_BITS_IN_REG       (6)

#define IMSIC_MAX_NUM_MSI       (2047)
#define IMSIC_MIN_MSI           (1)
#define IMSIC_NUM_MSI           (CONFIG_MAX_IRQ_PER_AGGREGATOR)

#define IMSIC_REG64_STEP                   (2)
#define IMSIC_REG64_WORD_OFFSET(src)       ((src >> AIA_REG64_POW2_BITS_IN_REG) * IMSIC_REG64_STEP)


#define IMSIC_REG_STEP              (IMSIC_REG64_STEP)
#define IMSIC_REG_MAX               (IMSIC_REG64_WORD_OFFSET(IMSIC_NUM_MSI))
#define IMSIC_REG_ALL_BITS_MASK     (0xFFFFFFFFFFFFFFFFULL)
#define IMSIC_REG_WORD_OFFSET(src)  (IMSIC_REG64_WORD_OFFSET(src))

#define AIA_REG64_BITS_IN_REG            (64)
#define AIA_REG64_BIT_OFFSET(src)        (src & (AIA_REG64_BITS_IN_REG - 1))
#define AIA_REG_BIT_OFFSET(src)          (AIA_REG64_BIT_OFFSET(src))

/**
 * @brief Base address for Incoming MSI Controller (IMSIC) in a machine-level domain
 */
#define IMSIC_BASE_ADDR (DT_REG_ADDR_U64(DT_NODELABEL(imsic)))

/**
 * @brief Incoming MSI Controller (IMSIC) interrupt file offset
 */
#define IMSIC_INT_FILE_OFFSET   (0x1000)

/**
 * @brief IMSIC CSRs, Machine-Level Window to Indirectly Accessed Registers
 */
#define IMSIC_CSR_MISELECT (0x350)
#define IMSIC_CSR_MIREG    (0x351)

/*
 * IMSIC indirect registers (with the miselect access) for using with imsic_read()
 * and imsic_write()
 */
/**
 * @brief Priorities for major interrupts
 * @details Priorities for major interrupts offset
 */
#define IMSIC_IPRIO_BASE  (0x30)
/**
 * @brief External interrupt delivery enable
 * @details  External interrupt delivery enable offset
 */
#define IMSIC_EIDELIVERY  (0x70)
/**
 * @brief External interrupt enable threshold
 *
 * @details External interrupt enable threshold offset
 */
#define IMSIC_EITHRESHOLD (0x72)
/**
 * @brief External interrupt-pending
 *
 * @details External interrupt-pending eip[0..63]
 */
#define IMSIC_EIP_BASE    (0x80)
/**
 * @brief External interrupt-enable
 *
 * @details External interrupt-enable eie[0..63]
 */
#define IMSIC_EIE_BASE    (0xC0)

/**
 * @brief write CSR register
 *
 * @param[in] reg - resister to write
 * @param[in] val - new value
 */
static inline void imsic_write(uint32_t reg, unsigned long val)
{
	csr_write(IMSIC_CSR_MISELECT, reg);
	csr_write(IMSIC_CSR_MIREG, val);
}

/**
 * @brief read CSR register
 *
 * @param[in] reg - resister to read
 *
 * @return register value
 */
static inline unsigned long imsic_read(uint32_t reg)
{
	csr_write(IMSIC_CSR_MISELECT, reg);
	return csr_read(IMSIC_CSR_MIREG);
}

/**
 * @brief Set enable bit in eie (external interrupt enable) indirect CSR
 *
 * @param[in] irq_id - interrupt ID
 */
static inline void imsic_set_eie(size_t irq_id)
{
	size_t reg = IMSIC_EIE_BASE + IMSIC_REG_WORD_OFFSET(irq_id);
	size_t bit = AIA_REG_BIT_OFFSET(irq_id);
	size_t eie = imsic_read(reg);

	eie |= BIT(bit);

	imsic_write(reg, eie);
}

/**
 * @brief Clear enable bit in eie (external interrupt enable) indirect CSR
 *
 * @param[in] irq_id - interrupt ID
 */
static inline void imsic_clr_eie(size_t irq_id)
{
	size_t reg = IMSIC_EIE_BASE + IMSIC_REG_WORD_OFFSET(irq_id);
	size_t bit = AIA_REG_BIT_OFFSET(irq_id);
	size_t eie = imsic_read(reg);

	eie &= ~BIT(bit);

	imsic_write(reg, eie);
}

/**
 * @brief Checks if enable bit is set in eie
 *
 * @param[in] irq_id - interrupt ID
 *
 * @return 1 if enable bit is set in eie, 0 otherwise
 */
static inline int imsic_is_eie_set(size_t irq_id)
{
	size_t reg = IMSIC_EIE_BASE + IMSIC_REG_WORD_OFFSET(irq_id);
	size_t bit = AIA_REG_BIT_OFFSET(irq_id);
	size_t eie = imsic_read(reg);

	return !!(eie & BIT(bit));
}

#define IMSIC_CSR_MTOPEI   (0x35C)

#define IMSIC_MTOPEI_INT_IDENTITY     GENMASK(26, 16)

/**
 * @brief read interrupt identity bits from the mtopei regsister
 *
 * @param[in] mtopei - mtopei value to read identity from
 *
 * @return interrupt identity from register value
 */
static inline uint64_t imsic_topei_identity(uint64_t mtopei)
{
	return FIELD_GET(IMSIC_MTOPEI_INT_IDENTITY, mtopei);
}

/**
 * @brief IMSIC initialization
 */
void riscv_imsic_init(void);

#endif /* ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_IMSIC_H_ */
