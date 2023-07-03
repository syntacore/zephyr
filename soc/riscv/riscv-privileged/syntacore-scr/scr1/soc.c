/*
 * Copyright (c) 2015-2022 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <soc.h>

#include <zephyr/arch/cpu.h>

void machine_trap_entry(void);

void scr1_mtvec_init(void)
{
    __asm__ volatile ( "csrw mtvec, %0"
                       ::
                       "r"(machine_trap_entry):
                     );
}

/**
 * @brief Do initialization proccess at boot time.
 * @return 0
 */
static int riscv32_scr1_init(void)
{
    /* Set up mtvec */
    scr1_mtvec_init();

    return 0;
}

SYS_INIT(riscv32_scr1_init, PRE_KERNEL_1, 0);
