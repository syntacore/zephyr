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
 * @file cluster_ctrl.c
 *
 * @brief Syntacore cluster control implementation
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include "cluster_ctrl.h"

/******************************************************************************
 *                              Defines section
 */
/* Value of 1 is a safe minimum */
#define SCR_JTAGIDCODE 1

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Enables secondary CPU cores, which are remain in halted state after
 * 		the cluster power on
 */
void cluster_ctrl_secondary_cores_enable(void)
{
	uint8_t num_cpus = arch_num_cpus();

	for (uint8_t cpu_num = 0; cpu_num < num_cpus; ++cpu_num) {
		/* Set Hart IDs */
		*(volatile uint64_t*)(CL_CPU_N_MHARTID(cpu_num)) = cpu_num;
		/* Set Jtag IDs */
		*(volatile uint64_t*)(CL_CPU_N_JTAGIDCODE(cpu_num)) = SCR_JTAGIDCODE;
	}

	/* Turn on CPUs */
	for (uint8_t cpu_num = 1; cpu_num < num_cpus; ++cpu_num) {
		*(volatile uint64_t*)(CL_CPU_N_CTRL(cpu_num)) =
			(CL_CPU_CTRL_CLOCK_EN_BIT | CL_CPU_CTRL_RESET_N_BIT);
	}
}
