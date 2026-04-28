# Copyright (C) 2026, Syntacore Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Common sources for all SCR9 cores 

zephyr_include_directories(../common)

zephyr_sources(
  ../common/mtimer.c
  ../common/memcheck.cpp
  ../common/cache.c
  ../common/scr_pmp.c
  ../common/ipi_iccm.c
)

zephyr_sources_ifdef(CONFIG_SCR_SYSTEM_HALT ../common/halt.c)
zephyr_sources_ifdef(CONFIG_SCR_MMU ../common/mmu.c)
zephyr_sources_ifdef(CONFIG_SCR_PMA ../common/pma.c)
zephyr_sources_ifdef(CONFIG_RISCV_HAS_PLIC ../common/plic.c)

# Common sources for SCR9 V1 cores
zephyr_sources_ifdef(CONFIG_CPU_SCR9_V1 ../common/iccm.c)

# Common sources for SCR9 non-V1 cores
zephyr_sources_ifndef(CONFIG_CPU_SCR9_V1 ../common/cluster_ctrl.c)

# Common sources for SCR9 V2 cores
zephyr_sources_ifdef(CONFIG_CPU_SCR9_V2 ../common/mrt.c)
zephyr_sources_ifdef(CONFIG_CPU_SCR9_V2 ../common/iccm_v2.c)
