/*
 * Copyright (c) 2017 Jean-Paul Etienne <fractalclone@gmail.com>
 * Copyright (C) Syntacore 2017-2022. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>

/* imports */
GTEXT(__start)
GTEXT(_isr_wrapper)

/* exports */
GTEXT(machine_trap_entry)

SECTION_FUNC(vectors, vinit)
	.option norvc;
machine_trap_entry:
    j do_irq_wrapper
do_irq_wrapper:
    tail _isr_wrapper
    .align 6
