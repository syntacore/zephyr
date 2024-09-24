# Copyright (C) 2022, Syntacore Ltd.
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

set(SOC_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld CACHE INTERNAL "")

# Common sources for the SCR9 family

zephyr_include_directories(../common)

zephyr_sources(
  ../common/cache.c
  ../common/plic.c
  ../common/memcheck.cpp
  ../common/scr_pmp.c
)

zephyr_sources_ifdef(CONFIG_SCR_MMU ../common/mmu.c)
zephyr_sources_ifdef(CONFIG_SCR_SYSTEM_HALT ../common/halt.c)

# Common sources for the SCR9 series

zephyr_include_directories_ifdef(CONFIG_SOC_SERIES_SCR9_L2 ./l2)

zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L2 ./l2/soc.c)
zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L2 ../common/iccm.c)

# Common sources for the SCR9 series

zephyr_include_directories_ifdef(CONFIG_SOC_SERIES_SCR9_L3 ./l3)

zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L3 ./l3/soc.c)
zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L3 ../common/iccm_v2.c)
zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L3 ../common/mrt.c)
zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR9_L3 ../common/cluster_ctrl.c)
