/**
 * Copyright (c) 2023 Syntacore
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file intc_scr_ipic.c
 * @author psk-sc
 * @brief Syntacore Integrated Programmable Interrupt Controller (IPIC) Interface
 *        for RISC-V processors
*/

#define DT_DRV_COMPAT riscv_scr_ipic

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/init.h>
#include <zephyr/arch/riscv/csr.h>
#include <zephyr/device.h>

#include <zephyr/sw_isr_table.h>
#include <zephyr/drivers/interrupt_controller/riscv_ipic.h>
#include <zephyr/irq.h>

/******************************************************************************
 *                              Defines section
 */
#define PLF_IPIC_STATIC_LINE_MAPPING CONFIG_IPIC_STATIC_LINE_MAPPING
#define PLF_IPIC_IRQ_LN_NUM (CONFIG_EXT_IPIC_IRQ_LN_NUM)
#define PLF_IPIC_IRQ_VEC_NUM PLF_IPIC_IRQ_LN_NUM
#define IPIC_IRQ_LN_VOID  PLF_IPIC_IRQ_LN_NUM

#define IPIC_IRQ_PENDING       (1 << 0)
#define IPIC_IRQ_ENABLE        (1 << 1)
#define IPIC_IRQ_LEVEL         (0 << 2)
#define IPIC_IRQ_EDGE          (1 << 2)
#define IPIC_IRQ_INV           (1 << 3)
#define IPIC_IRQ_MODE_MASK     (3 << 2)
#define IPIC_IRQ_CLEAR_PENDING IPIC_IRQ_PENDING

#define IPIC_IRQ_IN_SERVICE (1 << 4) // RO
#define IPIC_IRQ_PRIV_MASK  (3 << 8)
#define IPIC_IRQ_PRIV_MMODE (3 << 8)
#define IPIC_IRQ_PRIV_SMODE (1 << 8)
#define IPIC_IRQ_LN_OFFS    (12)

#ifndef PLF_IPIC_MBASE
#define PLF_IPIC_MBASE (0xbf0)
#endif
#define IPIC_CISV  (PLF_IPIC_MBASE + 0)
#define IPIC_CICSR (PLF_IPIC_MBASE + 1)
#define IPIC_IPR   (PLF_IPIC_MBASE + 2)
#define IPIC_ISVR  (PLF_IPIC_MBASE + 3)
#define IPIC_EOI   (PLF_IPIC_MBASE + 4)
#define IPIC_SOI   (PLF_IPIC_MBASE + 5)
#define IPIC_IDX   (PLF_IPIC_MBASE + 6)
#define IPIC_ICSR  (PLF_IPIC_MBASE + 7)
#define IPIC_IER   (PLF_IPIC_MBASE + 8)
#define IPIC_IMAP  (PLF_IPIC_MBASE + 9)
#define IPIC_VOID_VEC 16

#define MK_IRQ_CFG(line, mode, flags) ((mode) | (flags) | ((line) << IPIC_IRQ_LN_OFFS))

/******************************************************************************
 *                              Interface implementation section
 */
static void ipic_irq_enable(unsigned irq_vec);

static void ipic_irq_disable(unsigned irq_vec);

static unsigned long ipic_soi(void);

static void ipic_eoi(void);

static void ipic_irq_reset(int irq_vec);

static int ipic_irq_setup(int irq_vec, int line, int mode, int flags);

static unsigned long ipic_irq_current_vector(void);

/**
 * @brief Interface to enable a riscv IPIC-specific interrupt line
 *        for IRQS level == 2
 *
 * @param [in] irq_num - IRQ number to enable
 */
void riscv_ipic_irq_enable(uint32_t irq_num)
{
    uint32_t key = irq_lock();

    ipic_irq_enable(irq_num);

    irq_unlock(key);
}

/**
 * @brief Interface to disable a riscv IPIC-specific interrupt line
 *        for IRQS level == 2
 *
 * @param [in] irq_num - IRQ number to enable
 */
void riscv_ipic_irq_disable(uint32_t irq_num)
{
    uint32_t key = irq_lock();

    ipic_irq_disable(irq_num);

    irq_unlock(key);
}

/**
 * @brief Check if a riscv IPIC-specific interrupt line is enabled
 *
 * @param [in] irq_num - IRQ number
 *
 * @return Integer 1 - if enabled, 0 - otherwise
 */
int riscv_ipic_irq_is_enabled(uint32_t irq_num)
{
    return (int)(irq_num & csr_read(IPIC_CISV) && !(irq_num & IPIC_VOID_VEC));
}

/**
 * @brief Handle IPIC external IRQ
 *
 * @param [in] arg - Additional argument to handle IRQ
 */
static void scr_ipic_irq_handler(const void *arg)
{
    unsigned long currentVector = ipic_soi()  + CONFIG_2ND_LVL_ISR_TBL_OFFSET;

    struct _isr_table_entry *ite;

    ite = (struct _isr_table_entry *)&_sw_isr_table[currentVector];
    if (likely(ite)) {
        ite->isr(ite->arg);
    }

    ipic_eoi();
}

static int scr_ipic_init(void)
{
    /* disable interrupts for any line */
    for ( int i = 0; i < PLF_IPIC_IRQ_VEC_NUM; i++) {
        ipic_irq_reset(i);
    }

    IRQ_CONNECT(RISCV_MACHINE_EXT_IRQ,
		    IPIC_IRQ_LEVEL,
		    scr_ipic_irq_handler,
		    NULL,
		    0);

    irq_enable(RISCV_MACHINE_EXT_IRQ);

    return 0;
}

void ipic_irq_enable(unsigned irq_vec)
{
    csr_write(IPIC_IDX, irq_vec);
    const unsigned long state =
        (csr_read(IPIC_ICSR) & ~IPIC_IRQ_PENDING) | IPIC_IRQ_ENABLE;
    csr_write(IPIC_ICSR, state);
}

void ipic_irq_disable(unsigned irq_vec)
{
    csr_write(IPIC_IDX, irq_vec);
    const unsigned long state =
        csr_read(IPIC_ICSR) & ~(IPIC_IRQ_ENABLE | IPIC_IRQ_PENDING);
    csr_write(IPIC_ICSR, state);
}

unsigned long ipic_soi(void)
{
    csr_write(IPIC_SOI, 0);
    return ipic_irq_current_vector();
}

void ipic_eoi(void)
{
    csr_write(IPIC_EOI, 0);
}

void ipic_irq_reset(int irq_vec)
{
    ipic_irq_setup(irq_vec, IPIC_IRQ_LN_VOID, IPIC_IRQ_PRIV_MMODE, IPIC_IRQ_CLEAR_PENDING);
}

int ipic_irq_setup(int irq_vec, int line, int mode, int flags)
{
    if (PLF_IPIC_STATIC_LINE_MAPPING || irq_vec < 0)
        irq_vec = line;
    csr_write(IPIC_IDX, irq_vec);
    csr_write(IPIC_ICSR, MK_IRQ_CFG(line, mode, flags | IPIC_IRQ_CLEAR_PENDING));

    return irq_vec;
}

unsigned long ipic_irq_current_vector(void)
{
    return csr_read(IPIC_CISV);
}

SYS_INIT(scr_ipic_init, PRE_KERNEL_1, 0);
