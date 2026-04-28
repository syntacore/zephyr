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
 * @file intc_aplic.c
 *
 * @author mk-sc
 *
 * @brief Advanced Platform Level Interrupt Controller (APLIC) driver
 *        for RISC-V processors
 */

#define DT_DRV_COMPAT riscv_aplic

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/init.h>
#include <zephyr/device.h>

#include <zephyr/devicetree/interrupt_controller.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/drivers/interrupt_controller/riscv_aplic.h>
#include <zephyr/irq.h>

#include "intc_aplic_priv.h"

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
#include "intc_imsic.h"
#endif /* CONFIG_RISCV_HAS_IMSIC */

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Enable specific interrupt line
 *
 * @param irq IRQ number to enable
 */
void riscv_aplic_irq_enable(uint32_t irq)
{
	uint32_t sourcecfg = FIELD_PREP(APLIC_SOURCECFG_SM, APLIC_SOURCECFG_SM_EDGE1);
	uint32_t key = irq_lock();

	aplic_set_sourcecfg(irq, sourcecfg);

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
	uint32_t msi_target = aplic_make_msi_target(arch_proc_id(), irq);

	aplic_set_target(irq, msi_target);
	imsic_set_eie(irq);
#endif /* CONFIG_RISCV_HAS_IMSIC */

	aplic_set_setienum(irq);

	irq_unlock(key);
}

/**
 * @brief Disable specific interrupt line
 *
 * @param irq IRQ number to enable
 */
void riscv_aplic_irq_disable(uint32_t irq)
{
	uint32_t key;

	key = irq_lock();

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
		imsic_clr_eie(irq);
#else
		aplic_set_clrienum(irq);
#endif /* CONFIG_RISCV_HAS_IMSIC */

	irq_unlock(key);
}

/**
 * @brief Check if a RISCV APLIC-specific interrupt line is enabled.
 *
 * This routine checks if a RISCV APLIC-specific interrupt line is enabled.
 * @param irq IRQ number to check
 *
 * @return 1 if IRQ line is enable, 0 otherwise
 */
int riscv_aplic_irq_is_enabled(uint32_t irq)
{
#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
		return aplic_is_irq_enabled(irq) && imsic_is_eie_set(irq);
#else
		return aplic_is_irq_enabled(irq);
#endif /* CONFIG_RISCV_HAS_IMSIC */
}

/**
 * @brief Set priority of a specific interrupt line.
 *
 * Makes sense only for APLIC direct delivery, since
 * for MSI delivery with IMSIC the priority is determined by interrupt
 * identity number
 *
 * @param irq IRQ number for which to set priority
 * @param priority Priority of IRQ to set to
 */
void riscv_aplic_set_priority(uint32_t irq, uint32_t priority)
{
	if (!IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)) {
		if (priority > APLIC_MAX_PRIO)
			priority = APLIC_MAX_PRIO;

		uint32_t target = aplic_get_target(irq);

		uint32_t direct_target =
			aplic_make_direct_target(FIELD_GET(APLIC_TARGET_HART_IDX, target), priority);

		aplic_set_target(irq, direct_target);
	}
}

/**
 * @brief Process IRQ request from external interrupt source.
 *
 * This routine gets pending interrupt with highest priority and
 * invokes a corresponding IRQ handler.
 *
 * @param arg Parameters passed to IRQ handler.
 */
static void aplic_irq_handler(const void *arg)
{
	struct _isr_table_entry *ite;
	uint32_t irq;

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
	/* Claim IRQ source number from mtopei */
	uint64_t mtopei = csr_swap(IMSIC_CSR_MTOPEI, 0);

	irq = imsic_topei_identity(mtopei);
#else
	/* Claim IRQ source number from interrupt delivery control structure */
	uint32_t claimi = aplic_get_idc_claimi(arch_proc_id());

	irq = FIELD_GET(APLIC_IDC_IRQ_ID, claimi);
#endif /* CONFIG_RISCV_HAS_IMSIC */

	/*
	 * Spurious interrupts with ID 0 are possible due to delay in communication
	 * between an APLIC and a hart. Such interrupts can be safely ignored.
	 * See RISC-V AIA V1.0-RC2 4.8.2 for details.
	 * TODO: check if it is possible to gracefully process such IRQs.
	 */
	if (irq == 0 || irq > APLIC_SOURCE_MAX)
		z_irq_spurious(NULL);

	irq += CONFIG_2ND_LVL_ISR_TBL_OFFSET;

	ite = (struct _isr_table_entry *)&_sw_isr_table[irq];
	ite->isr(ite->arg);
}

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
/**
 * @brief Configure APLIC for MSI delivery
 */
static void aplic_init_msi(void)
{
	uint32_t val = FIELD_PREP(APLIC_DOMAINCFG_DM, APLIC_DOMAINCFG_DM_MSI);

	aplic_set_domaincfg(val);

	struct aplic_msicfg_data msicfg = {
		.base_addr = IMSIC_BASE_ADDR,
		.lhxw = APLIC_LHXW_M,
		.lhxs = APLIC_LHXS_M,
		.hhxw = APLIC_HHXW_M,
		.hhxs = APLIC_HHXS_M
	};

	aplic_write_msiconfig(&msicfg);
}
#else
/**
 * @brief Configure APLIC for direct delivery of interrupts
 */
static void aplic_init_direct_delivery(void)
{
	/* Initialize IDC structures for all harts */
	for (int hart = 0; hart < arch_num_cpus(); hart++) {
		aplic_set_idc_idelivery(hart, APLIC_IDC_IDELIVERY_DISABLE);
	}

	/*
	 * Enable delivery of IRQs for hart 0.
	 * Disable priority threshold to deliver all IRQs
	 */
	aplic_set_idc_idelivery(0, APLIC_IDC_IDELIVERY_ENABLE);
	aplic_set_idc_ithreshold(0, APLIC_IDC_ITHRESHOLD_DISABLE);
}
#endif /* CONFIG_RISCV_HAS_IMSIC */

/**
 * @brief Set initial APLIC configuration
 */
static void aplic_init(void)
{
	/* Set inactive mode and disable delivery for all interrupts */
	for (int src = APLIC_SOURCE_MIN; src <= APLIC_SOURCE_MAX; src++) {
		aplic_set_sourcecfg(src, APLIC_SOURCECFG_SM_INACTIVE);
	}

	/* Clear all interrupts */
	for (int reg_num = APLIC_MIN_IRQ_REG; reg_num <= APLIC_MAX_IRQ_REG; reg_num++) {
		aplic_set_clrie(reg_num, APLIC_CLRIE_CLR_ALL);
	}

	/* Configure delivery mode */
#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
	aplic_init_msi();
#else
	aplic_init_direct_delivery();
#endif /* CONFIG_RISCV_HAS_IMSIC */
}

/**
 * @brief Initialize Advanced Interrupt Architecture: APLIC and possibly IMSIC
 *
 * @retval 0 on success.
 */
static int aia_init(const struct device *dev)
{
	/* Disable all APLIC interrupts initially */
	aplic_set_domaincfg(0U);

	aplic_init();

#if IS_ENABLED(CONFIG_RISCV_HAS_IMSIC)
	riscv_imsic_init();
#endif /* CONFIG_RISCV_HAS_IMSIC */

	/* Enable APLIC interrupts */
	uint32_t domaincfg = aplic_get_domaincfg();

	domaincfg |= APLIC_DOMAINCFG_IE;
	aplic_set_domaincfg(domaincfg);

	/* Setup IRQ handler for APLIC driver */
	IRQ_CONNECT(RISCV_IRQ_MEXT,
			0,
			aplic_irq_handler,
			NULL,
			0);

	/* Enable External IRQ for the hart */
	irq_enable(RISCV_IRQ_MEXT);

	ARG_UNUSED(dev);
	return 0;
}

#define APLIC_INTC_DEVICE_INIT(n)                    \
	IRQ_PARENT_ENTRY_DEFINE(                               \
		aplic##n, DEVICE_DT_INST_GET(n), DT_INST_IRQN(n),  \
		INTC_INST_ISR_TBL_OFFSET(n),                       \
		DT_INST_INTC_GET_AGGREGATOR_LEVEL(n));             \
	DEVICE_DT_INST_DEFINE(n, &aia_init, NULL,  \
		NULL, NULL,                                 \
		PRE_KERNEL_1, CONFIG_INTC_INIT_PRIORITY,    \
		NULL);

DT_INST_FOREACH_STATUS_OKAY(APLIC_INTC_DEVICE_INIT)
