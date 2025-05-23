# Copyright (C) 2024, Syntacore Ltd.
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

# Common sources for the SCR3 family

zephyr_include_directories(../common)

zephyr_sources(
  ../common/cache.c
  ../common/soc.c
)

zephyr_sources_ifdef(CONFIG_SCR_MPU ../common/mpu.c)
zephyr_sources_ifdef(CONFIG_SCR_SYSTEM_HALT ../common/halt.c)

# Common sources for the SCR3 series

zephyr_include_directories_ifdef(CONFIG_SOC_SERIES_SCR3 ./l1)


# Common sources for the SCR3 L2 series

zephyr_include_directories_ifdef(CONFIG_SOC_SERIES_SCR3_L2 ./l2)

zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR3_L2 ../common/plic.c)
zephyr_sources_ifdef(CONFIG_SOC_SERIES_SCR3_L2 ../common/iccm.c)
