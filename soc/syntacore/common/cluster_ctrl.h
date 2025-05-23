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
 * @file cluster_ctrl.h
 *
 * @brief Syntacore Cluster Control interface
 *
 * @author amr-sc
 */

#ifndef SCR_CLUSTER_CTRL_H
#define SCR_CLUSTER_CTRL_H

/******************************************************************************
 *                              Includes section
 */
#include <soc.h>

/******************************************************************************
 *                              Defines section
 */
#define PLF_CLUSTER_CPU_BASE            (PLF_CLUSTER_CFG_BASE + 4 * 1024)

#define CL_CPU_DESCR                    (PLF_CLUSTER_CPU_BASE + 0)
/* CPU0 Control Register */
#define CL_CPU_0_CTRL                   (PLF_CLUSTER_CPU_BASE + 0x40)
/* CPU0 MHARTID Register */
#define CL_CPU_0_MHARTID                (PLF_CLUSTER_CPU_BASE + 0x48)
/* CPU0 PC to start after reset */
#define CL_CPU_0_RESETPC                (PLF_CLUSTER_CPU_BASE + 0x50)
/* CPU0 JTAG ID code */
#define CL_CPU_0_JTAGIDCODE             (PLF_CLUSTER_CPU_BASE + 0x58)

#define CL_CPU_N_CTRL(N)                (CL_CPU_0_CTRL + (N) * 0x40)
#define CL_CPU_N_MHARTID(N)             (CL_CPU_0_MHARTID + (N) * 0x40)
#define CL_CPU_N_RESETPC(N)             (CL_CPU_0_RESETPC + (N) * 0x40)
#define CL_CPU_N_JTAGIDCODE(N)          (CL_CPU_0_JTAGIDCODE + (N) * 0x40)

/* This bit correpsonds to rst_n pin of CL2 */
#define CL_CPU_CTRL_RESET_N_BIT                 (1 << 0)
/* CL2 is ready after reset */
#define CL_CPU_CTRL_RDY_BIT                     (1 << 8)
/* CL2 CPU debug reset status: 0 - under reset, 1 - reset done */
#define CL_CPU_CTRL_DBG_RESET_STATUS_BIT        (1 << 16)
/* Enable signal for clk pin */
#define CL_CPU_CTRL_CLOCK_EN_BIT                (1 << 24)
/* Block CPU to initiate new memory transactions */
#define CL_CPU_CTRL_BLK_BIT                     (1ULL << 32)
 /* Flag that CPU has stopped to initiate new memory transactions */
#define CL_CPU_CTRL_BLK_DONE                    (1ULL << 40)

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Enables secondary CPU cores, which are remain in halted state after
 * 		the cluster power on
 */
void cluster_ctrl_secondary_cores_enable(void);

#endif /* SCR_CLUSTER_CTRL_H */
