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
 * @file memory_map.h
 *
 * @brief SoC memory map for the L3 cluster
 *
 * @author amr-sc
 */

#ifndef __MEMORY_MAP_H_
#define __MEMORY_MAP_H_

/******************************************************************************
 *                              Includes section
 */
#include <soc.h>
#include "../../common/mrt.h"

/******************************************************************************
 *                              Defines section
 */
#define PLF_MEM_MAP                                                     \
    {PLF_CLUSTER_CFG_BASE, PLF_CLUSTER_CFG_SIZE, PLF_CLUSTER_CFG_ATTR, PLF_CLUSTER_CFG_NAME, CLUSTER_CSR_MPORT},  \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME, PLF_DEFAULT_MPORT},           \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME, PLF_DEFAULT_MPORT},   \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME, PLF_DEFAULT_MPORT},       \
    {PLF_MTIMER_BASE, PLF_MTIMER_SIZE, PLF_MTIMER_ATTR, PLF_MTIMER_NAME, CLUSTER_CSR_MPORT},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME, CLUSTER_CSR_MPORT},       \
    {PLF_ICCM_BASE, PLF_ICCM_SIZE, PLF_ICCM_ATTR, PLF_ICCM_NAME, CLUSTER_CSR_MPORT}, \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME, PLF_DEFAULT_MPORT}

#endif /* __MEMORY_MAP_H_ */
