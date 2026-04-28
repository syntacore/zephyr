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
 * @file pma.c
 *
 * @brief Syntacore Phusical Memory Attributes support
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <soc.h>
#include "pma.h"

#if IS_ENABLED(CONFIG_SCR_PMA)

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Performs PMA setup
 */
void scr_pma_setup(void)
{
	/* Setup main memory as cached weakly-ordered idempotent */
	pma_configure_region(CONFIG_SCR_PMA_ENTRIES_COUNT - 3, PLF_MEM_BASE,
		PLF_MEM_SIZE, SCR_PMA_MEM_REGION_C_WO);
	pma_configure_region(CONFIG_SCR_PMA_ENTRIES_COUNT - 2, PLF_OCRAM_BASE,
		PLF_OCRAM_SIZE, SCR_PMA_MEM_REGION_C_WO);

	/* All the remaining memory configured as IO */
	pma_configure_region(CONFIG_SCR_PMA_ENTRIES_COUNT - 1, ALL_MEMORY_BASE,
		ALL_MEMORY_SIZE, SCR_PMA_MEM_REGION_IO);
}

#endif /* CONFIG_SCR_PMA */
