/*
 * @file plic.c
 *
 * @brief Syntacore fixup for PLIC
 *
 * @author amr-sc
 *
 * Copyright (c) 2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/riscv/csr.h>
#include <soc.h>
#include "cache.h"

#if IS_ENABLED(CONFIG_RISCV_HAS_PLIC)

/******************************************************************************
 *                              Defines section
 */
#if DT_HAS_COMPAT_STATUS_OKAY(sifive_plic_1_0_0)
#define PLIC_NODE DT_NODELABEL(plic)
#define SCR_PLIC_BASE UINT64_C(DT_REG_ADDR_BY_NAME(PLIC_NODE, prio))
#else
#error "No SCR PLIC devicetree information"
#endif

#define SCR_PLIC_MAX_LINES_NUMBER	128
#define SCR_PLIC_MODE_BASE_OFFSET	0x1f0000
#define SCR_PLIC_MODE_REG_WIDTH		0x04

#define SCR_PLIC_INTLINE_UART	DT_IRQN(DT_NODELABEL(uart))

/******************************************************************************
 *                              Types section
 */
enum scr_plic_mode {
	SCR_PLIC_SRC_MODE_OFF		= 0,
	SCR_PLIC_SRC_MODE_LEVEL_HIGH	= 1,
	SCR_PLIC_SRC_MODE_LEVEL_LOW	= 2,
	SCR_PLIC_SRC_MODE_EDGE_RISING	= 3,
	SCR_PLIC_SRC_MODE_EDGE_FALLING	= 4,
	SCR_PLIC_SRC_MODE_EDGE_BOTH	= 5,
	SCR_PLIC_SRC_MODE_MAX		= SCR_PLIC_SRC_MODE_EDGE_BOTH,
};

/******************************************************************************
 *                              Static variables section
 */
static const struct {
	uint32_t line;
	uint32_t mode;
} irq_modes[] = {
	{SCR_PLIC_INTLINE_UART, SCR_PLIC_SRC_MODE_LEVEL_HIGH},
};

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Obtains actual IRQ number encoded in Zephyr's internal representation
 *
 * @param [in] encoded_irq - IRQ number in Zephyr's internal representation
 *
 * @return Actual IRQ number
 */
static inline uint32_t plic_decode_irq(uint32_t encoded_irq)
{
	switch (irq_get_level(encoded_irq)) {

#ifdef CONFIG_3RD_LEVEL_INTERRUPTS
	case 3:
		return irq_from_level_3(encoded_irq);
#endif /* CONFIG_3RD_LEVEL_INTERRUPTS */
	case 2:
		return irq_from_level_2(encoded_irq);
	default:
		return encoded_irq;

	}
}

/**
 * @brief Sets PLIC Source Mode
 *
 * @param [in] encoded_irq - IRQ number in Zephyr's internal representation
 * @param [in] mode - IRQ triggering mode
 */
static inline void plic_set_mode(uint32_t encoded_irq, enum scr_plic_mode mode)
{
	uint32_t irq = plic_decode_irq(encoded_irq);
	volatile uint32_t *source_mode = (void *)(uintptr_t)(SCR_PLIC_BASE +
		SCR_PLIC_MODE_BASE_OFFSET + SCR_PLIC_MODE_REG_WIDTH * irq);

	*source_mode = mode;
}

/**
 * @brief Fixes-up PLIC registers which couldn't be initialized
 *		with existing Zephyr API
 *
 * @return 0
 */
static int scr_plic_fixup(void)
{
	int i;

	/*
	 * Bring Source Mode values to default state
	 * Line numbering starts from 1, line 0 is hardwired to zero
	 */
	for (i = 1; i < SCR_PLIC_MAX_LINES_NUMBER; i++)
		plic_set_mode(i, SCR_PLIC_SRC_MODE_OFF);

	/* Setup desired Source Mode values */
	for (i = 0; i < ARRAY_SIZE(irq_modes); i++)
		plic_set_mode(irq_modes[i].line, irq_modes[i].mode);

	return 0;
}

SYS_INIT(scr_plic_fixup, PRE_KERNEL_2, 0);

#endif /* RISCV_HAS_PLIC */
