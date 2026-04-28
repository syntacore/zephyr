/*
 * Copyright (C) 2024, Syntacore Ltd.
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

/**
 * @file linker.ld
 * @brief Linker command/script file
 *
 * Generic Linker script for the SCR7-based SoC.
 */

#include <zephyr/arch/riscv/common/linker.ld>

#if IS_ENABLED(CONFIG_SCR_PMP_ADDR_SANITIZER)
#include "soc.h"
#include <scr_pmp.h>
#define PLF_MEM_BASE_ADDR    (DT_REG_ADDR(DT_NODELABEL(ddr)))
#define PLF_MEM_SIZE_ADDR    (DT_REG_SIZE(DT_NODELABEL(ddr)))

SECTIONS
{
    __unused_mem_size  = PLF_MEM_BASE_ADDR + PLF_MEM_SIZE_ADDR - _end;
    __unused_mem_start = ALIGN(_end, SCR_MEM_PROT_MIN_REGION_SIZE);
}
#endif /* CONFIG_SCR_PMP_ADDR_SANITIZER */