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
 * @file intc_scr_ipic.c
 *
 * @author psk-sc
 *
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

#include <zephyr/devicetree/interrupt_controller.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/drivers/interrupt_controller/riscv_ipic.h>
#include <zephyr/irq.h>

/******************************************************************************
 *                              Defines section
 */
/*
 * IPIC supports either 16 or 32 IRQs.
 * First 12 entries in _sw_isr_table are reserved for standard RISC-V
 * exception codes followed by external interrupts supplied by IPIC.
 * CONFIG_NUM_IRQS includes both IPIC IRQs and these 12 entries
 */
BUILD_ASSERT(CONFIG_NUM_IRQS == 28 || CONFIG_NUM_IRQS == 44, \
	"Invalid CONFIG_NUM_IRQS for IPIC");
#define IPIC_IRQS_NUM (CONFIG_NUM_IRQS - CONFIG_2ND_LVL_ISR_TBL_OFFSET)
BUILD_ASSERT(IPIC_IRQS_NUM == 16 || IPIC_IRQS_NUM == 32, \
	"IPIC supports either 16 or 32 IRQs");

#define IPIC_VOID_VEC  IPIC_IRQS_NUM

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

/* ICSR */
#define IPIC_ICSR_IP_SHIFT (0)
#define IPIC_ICSR_IP_CLEAR (1 << IPIC_ICSR_IP_SHIFT)

#define IPIC_ICSR_IE_SHIFT        (1)
#define IPIC_ICSR_IE_IRQ_ENABLED  (1 << IPIC_ICSR_IE_SHIFT)
#define IPIC_ICSR_IE_IRQ_DISABLED (0 << IPIC_ICSR_IE_SHIFT)

#define IPIC_ICSR_IM_SHIFT (2)
#define IPIC_ICSR_IM_LEVEL (0 << IPIC_ICSR_IM_SHIFT)
#define IPIC_ICSR_IM_EDGE  (1 << IPIC_ICSR_IM_SHIFT)

#define IPIC_ICSR_INV_SHIFT    (3)
#define IPIC_ICSR_INV_DISABLED (0 << IPIC_ICSR_INV_SHIFT)
#define IPIC_ICSR_INV_ENABLED  (1 << IPIC_ICSR_INV_SHIFT)

#define IPIC_ICSR_PRV_SHIFT  (8)
#define IPIC_ICSR_PRIV_MMODE (3 << IPIC_ICSR_PRV_SHIFT)

#define IPIC_ICSR_LN_SHIFT (12)
#define IPIC_ICSR_LN(ln)   ((ln) << IPIC_ICSR_LN_SHIFT)

/******************************************************************************
 *                              Interface implementation section
 */
static void ipic_irq_enable(unsigned irq_vec);
static void ipic_irq_disable(unsigned irq_vec);
static uint32_t ipic_soi(void);
static void ipic_eoi(void);
static void ipic_irq_reset(int irq_vec);
static uint32_t ipic_irq_current_vector(void);

/**
 * @brief Interface to enable a riscv IPIC-specific interrupt line
 *        for IRQS level == 2
 *
 * @param [in] irq_num - IRQ number to enable
 */
void riscv_custom_ic_irq_enable(uint32_t irq_num)
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
void riscv_custom_ic_irq_disable(uint32_t irq_num)
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
int riscv_custom_ic_irq_is_enabled(uint32_t irq_num)
{
    csr_write(IPIC_IDX, irq_num);
    return !!(csr_read(IPIC_ICSR) & IPIC_ICSR_IE_IRQ_ENABLED);
}

/**
 * @brief Handle IPIC external IRQ
 *
 * @param [in] arg - Additional argument to handle IRQ
 */
static void scr_ipic_irq_handler(const void *arg)
{
    uint32_t currentVector = ipic_soi();

    /* Filter spurious interrupt */
    if (unlikely(currentVector == IPIC_VOID_VEC)) {
    	goto eoi;
    }

    int isr_offset = currentVector + CONFIG_2ND_LVL_ISR_TBL_OFFSET;
    struct _isr_table_entry *ite = (struct _isr_table_entry *)&_sw_isr_table[isr_offset];

    if (likely(ite)) {
        ite->isr(ite->arg);
    }

eoi:
    ipic_eoi();
}

static int scr_ipic_init(const struct device *dev)
{
	ARG_UNUSED(dev);

    /* Disable interrupts for all lines */
    for ( int i = 0; i < IPIC_IRQS_NUM; i++) {
        ipic_irq_reset(i);
    }

    IRQ_CONNECT(RISCV_IRQ_MEXT,
                0,
                scr_ipic_irq_handler,
                NULL,
                0);

    irq_enable(RISCV_IRQ_MEXT);

    return 0;
}

static void ipic_irq_enable(unsigned irq_vec)
{
    csr_write(IPIC_IDX, irq_vec);

    uint32_t icsr = csr_read(IPIC_ICSR);

    /* We shouldn't alter ICSR IP state, so write 0 to it */
    icsr &= ~(IPIC_ICSR_IP_CLEAR);
    icsr |= IPIC_ICSR_IE_IRQ_ENABLED;

    csr_write(IPIC_ICSR, icsr);
}

static void ipic_irq_disable(unsigned irq_vec)
{
    csr_write(IPIC_IDX, irq_vec);

    uint32_t icsr = csr_read(IPIC_ICSR);

    /* We shouldn't alter ICSR IP state, so write 0 to it */
    icsr &= ~(IPIC_ICSR_IP_CLEAR | IPIC_ICSR_IE_IRQ_ENABLED);

    csr_write(IPIC_ICSR, icsr);
}

static uint32_t ipic_soi(void)
{
    csr_write(IPIC_SOI, 0);
    return ipic_irq_current_vector();
}

static void ipic_eoi(void)
{
    csr_write(IPIC_EOI, 0);
}

static void ipic_irq_reset(int irq_vec)
{
	uint32_t icsr = IPIC_ICSR_IP_CLEAR | IPIC_ICSR_IE_IRQ_DISABLED | IPIC_ICSR_IM_LEVEL |
		IPIC_ICSR_INV_DISABLED | IPIC_ICSR_PRIV_MMODE |	IPIC_ICSR_LN(irq_vec);

    csr_write(IPIC_IDX, irq_vec);
    csr_write(IPIC_ICSR, icsr);
}

static uint32_t ipic_irq_current_vector(void)
{
    return csr_read(IPIC_CISV);
}

#define IPIC_INTC_DEVICE_INIT(n)                    \
	IRQ_PARENT_ENTRY_DEFINE(                                                               \
		ipic##n, DEVICE_DT_INST_GET(n), DT_INST_IRQN(n),                                   \
		INTC_INST_ISR_TBL_OFFSET(n),                                                       \
		DT_INST_INTC_GET_AGGREGATOR_LEVEL(n));                                             \
	DEVICE_DT_INST_DEFINE(n, &scr_ipic_init, NULL,  \
		NULL, NULL,                                 \
		PRE_KERNEL_1, CONFIG_INTC_INIT_PRIORITY,    \
		NULL);

DT_INST_FOREACH_STATUS_OKAY(IPIC_INTC_DEVICE_INIT)
