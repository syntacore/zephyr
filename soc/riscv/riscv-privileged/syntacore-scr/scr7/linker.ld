/*
 * Copyright (c) 2022-2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file linker.ld
 * @brief Linker command/script file
 *
 * Generic Linker script for the SCR7-based SoC.
 */

#include <zephyr/arch/riscv/common/linker.ld>

#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
#include "soc.h"
#include "../common/scr_pmp.h"
#define PLF_MEM_BASE_ADDR    (DT_REG_ADDR(DT_NODELABEL(ddr)))
#define PLF_MEM_SIZE_ADDR    (DT_REG_SIZE(DT_NODELABEL(ddr)))

SECTIONS
{
    __unused_mem_size  = PLF_MEM_BASE_ADDR + PLF_MEM_SIZE_ADDR - _end;
    __unused_mem_start = ALIGN(_end, SCR_MEM_PROT_MIN_REGION_SIZE);
}
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */