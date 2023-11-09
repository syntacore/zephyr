/*
 * Copyright (c) 2017 Jean-Paul Etienne <fractalclone@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file configuration macros for riscv SOCs supporting the riscv
 *       privileged architecture specification
 */

#ifndef __SOC_COMMON_H_
#define __SOC_COMMON_H_

/* IRQ numbers */
#define RISCV_MACHINE_SOFT_IRQ       3  /* Machine Software Interrupt */
#define RISCV_MACHINE_EXT_IRQ        11 /* Machine External Interrupt */

/* ECALL Exception numbers */
#define SOC_MCAUSE_ECALL_EXP            11 /* Machine ECALL instruction */
#define SOC_MCAUSE_SUPERVISOR_ECALL_EXP 9  /* Supervisor ECALL instruction */
#define SOC_MCAUSE_USER_ECALL_EXP       8  /* User ECALL instruction */
/* Misaligned access exceptions */
#define SOC_MCAUSE_LD_MISALIGNED_EXP    4  /* Machine Load address misaligned exception */
#define SOC_MCAUSE_ST_MISALIGNED_EXP    6  /* Machine Store/AMO address misaligned exception */

/* SOC-specific MCAUSE bitfields */
#ifdef CONFIG_64BIT
/* Interrupt Mask */
#define SOC_MCAUSE_IRQ_MASK          (1 << 63)
#else
/* Interrupt Mask */
#define SOC_MCAUSE_IRQ_MASK          (1 << 31)
#endif

/* Exception code Mask */
#define SOC_MCAUSE_EXP_MASK          CONFIG_RISCV_SOC_MCAUSE_EXCEPTION_MASK

#ifndef _ASMLANGUAGE

#include <zephyr/drivers/interrupt_controller/riscv_clic.h>
#include <zephyr/drivers/interrupt_controller/riscv_plic.h>
#include <zephyr/drivers/interrupt_controller/riscv_ipic.h>

#if defined(CONFIG_RISCV_SOC_INTERRUPT_INIT)
void soc_interrupt_init(void);
#endif

/* RISC-V custom interrupt controller API, see CONFIG_RISCV_HAS_CUSTOM_IC */

/**
 * @brief Enable an interrupt line for RISC-V custom interrupt controller.
 *
 * This routine enables an interrupt line for RISC-V custom interrupt controller.
 * Should be implemented by custom interrupt controller for 2nd level IRQs.
 *
 * @param irq 2nd level IRQ number to enable
 */
void riscv_custom_ic_irq_enable(uint32_t irq);

/**
 * @brief Disable an interrupt line for RISC-V custom interrupt controller.
 *
 * This routine disables an interrupt line for RISC-V custom interrupt controller.
 * Should be implemented by custom interrupt controller for 2nd level IRQs.
 *
 * @param irq 2nd level IRQ number to disable
 */
void riscv_custom_ic_irq_disable(uint32_t irq);

/**
 * @brief Check if an interrupt line for RISC-V custom interrupt controller is enabled
 *
 * This routine checks if an interrupt line for RISC-V custom interrupt
 * controller is enabled.
 * Should be implemented by custom interrupt controller for 2nd level IRQs.
 *
 * @param irq 2nd level IRQ number to check
 *
 * @return 1 - if 2nd level IRQ is enabled; 0 - otherwise
 */
int riscv_custom_ic_irq_is_enabled(uint32_t irq);

#endif /* !_ASMLANGUAGE */

#endif /* __SOC_COMMON_H_ */
