/*
 * Copyright (c) 2017 Jean-Paul Etienne <fractalclone@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief interrupt management code for riscv SOCs supporting the riscv
	  privileged architecture specification
 */
#include <zephyr/irq.h>
#include <zephyr/irq_multilevel.h>

#include <zephyr/drivers/interrupt_controller/riscv_clic.h>
#include <zephyr/drivers/interrupt_controller/riscv_plic.h>
#include <zephyr/drivers/interrupt_controller/riscv_aplic.h>
#include <zephyr/drivers/interrupt_controller/riscv_ipic.h>

#include <zephyr/devicetree.h>

#if defined(CONFIG_RISCV_HAS_CLIC)

void arch_irq_enable(unsigned int irq)
{
	riscv_clic_irq_enable(irq);
}

void arch_irq_disable(unsigned int irq)
{
	riscv_clic_irq_disable(irq);
}

int arch_irq_is_enabled(unsigned int irq)
{
	return riscv_clic_irq_is_enabled(irq);
}

void z_riscv_irq_priority_set(unsigned int irq, unsigned int prio, uint32_t flags)
{
	riscv_clic_irq_priority_set(irq, prio, flags);
}

void z_riscv_irq_vector_set(unsigned int irq)
{
#if defined(CONFIG_CLIC_SMCLICSHV_EXT)
	riscv_clic_irq_vector_set(irq);
#else
	ARG_UNUSED(irq);
#endif
}

#elif defined(CONFIG_RISCV_HAS_APLIC)

void arch_irq_enable(unsigned int irq)
{
	uint32_t mie;
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
		irq = irq_from_level_2(irq);
		riscv_aplic_irq_enable(irq);
		return;
	}

	/*
	 * CSR mie register is updated using atomic instruction csrrs
	 * (atomic read and set bits in CSR register)
	 */
	mie = csr_read_set(mie, 1 << irq);
}

void arch_irq_disable(unsigned int irq)
{
	uint32_t mie;
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
		irq = irq_from_level_2(irq);
		riscv_aplic_irq_disable(irq);
		return;
	}

	/*
	 * Use atomic instruction csrrc to disable device interrupt in mie CSR.
	 * (atomic read and clear bits in CSR register)
	 */
	mie = csr_read_clear(mie, 1 << irq);
}

int arch_irq_is_enabled(unsigned int irq)
{
	uint32_t mie;
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
		irq = irq_from_level_2(irq);
		return riscv_aplic_irq_is_enabled(irq);
	}

	mie = csr_read(mie);

	return !!(mie & (1 << irq));
}

void z_riscv_irq_priority_set(unsigned int irq, unsigned int prio, uint32_t flags)
{
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
		irq = irq_from_level_2(irq);
		riscv_aplic_set_priority(irq, prio);
	}
}

#else /* PLIC + HLINT/CLINT or HLINT/CLINT or custom interrupt controller */

#if DT_HAS_COMPAT_STATUS_OKAY(sifive_plic_1_0_0)
#define riscv_ic_irq_enable riscv_plic_irq_enable
#define riscv_ic_irq_disable riscv_plic_irq_disable
#define riscv_ic_irq_is_enabled riscv_plic_irq_is_enabled
#elif DT_HAS_COMPAT_STATUS_OKAY(riscv_scr_ipic)
#define riscv_ic_irq_enable riscv_ipic_irq_enable
#define riscv_ic_irq_disable riscv_ipic_irq_disable
#define riscv_ic_irq_is_enabled riscv_ipic_irq_is_enabled
#endif

void arch_irq_enable(unsigned int irq)
{
	uint32_t mie;

#if defined(CONFIG_2ND_LEVEL_INTERRUPTS)
#if defined(CONFIG_RISCV_HAS_PLIC) || defined(CONFIG_RISCV_HAS_CUSTOM_IC)
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
#if defined(CONFIG_RISCV_HAS_PLIC)
		riscv_plic_irq_enable(irq);
#else
		irq = irq_from_level_2(irq);
		riscv_custom_ic_irq_enable(irq);
#endif /* CONFIG_RISCV_HAS_PLIC */
		return;
	}
#endif /* CONFIG_RISCV_HAS_PLIC || CONFIG_RISCV_HAS_CUSTOM_IC */
#endif /* CONFIG_2ND_LEVEL_INTERRUPTS */

	/*
	 * CSR mie register is updated using atomic instruction csrrs
	 * (atomic read and set bits in CSR register)
	 */
	mie = csr_read_set(mie, 1 << irq);
}

void arch_irq_disable(unsigned int irq)
{
	uint32_t mie;

#if defined(CONFIG_2ND_LEVEL_INTERRUPTS)
#if defined(CONFIG_RISCV_HAS_PLIC) || defined(CONFIG_RISCV_HAS_CUSTOM_IC)
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
#if defined(CONFIG_RISCV_HAS_PLIC)
		riscv_plic_irq_disable(irq);
#else
		irq = irq_from_level_2(irq);
		riscv_custom_ic_irq_disable(irq);
#endif /* CONFIG_RISCV_HAS_PLIC */
		return;
	}
#endif /* CONFIG_RISCV_HAS_PLIC || CONFIG_RISCV_HAS_CUSTOM_IC */
#endif /* CONFIG_2ND_LEVEL_INTERRUPTS */

	/*
	 * Use atomic instruction csrrc to disable device interrupt in mie CSR.
	 * (atomic read and clear bits in CSR register)
	 */
	mie = csr_read_clear(mie, 1 << irq);
}

int arch_irq_is_enabled(unsigned int irq)
{
	uint32_t mie;

#if defined(CONFIG_2ND_LEVEL_INTERRUPTS)
#if defined(CONFIG_RISCV_HAS_PLIC) || defined(CONFIG_RISCV_HAS_CUSTOM_IC)
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
#if defined(CONFIG_RISCV_HAS_PLIC)
		return riscv_plic_irq_is_enabled(irq);
#else
		irq = irq_from_level_2(irq);
		return riscv_custom_ic_irq_is_enabled(irq);
#endif /* CONFIG_RISCV_HAS_PLIC */
	}
#endif /* CONFIG_RISCV_HAS_PLIC || CONFIG_RISCV_HAS_CUSTOM_IC */
#endif /* CONFIG_2ND_LEVEL_INTERRUPTS */

	mie = csr_read(mie);

	return !!(mie & (1 << irq));
}

#if defined(CONFIG_RISCV_HAS_PLIC)
void z_riscv_irq_priority_set(unsigned int irq, unsigned int prio, uint32_t flags)
{
	unsigned int level = irq_get_level(irq);

	if (level == 2) {
		riscv_plic_set_priority(irq, prio);
	}
}
#endif /* CONFIG_RISCV_HAS_PLIC */
#endif /* CONFIG_RISCV_HAS_CLIC */

#if defined(CONFIG_RISCV_SOC_INTERRUPT_INIT)
__weak void soc_interrupt_init(void)
{
	/* ensure that all interrupts are disabled */
	(void)arch_irq_lock();

	csr_write(mie, 0);
	csr_write(mip, 0);
}
#endif
