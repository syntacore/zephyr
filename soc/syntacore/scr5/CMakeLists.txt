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

set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/include/zephyr/arch/riscv/common/linker.ld CACHE INTERNAL "")

# Common sources for the SCR5 family
zephyr_include_directories(../common)
zephyr_include_directories(${ZEPHYR_BASE}/arch/riscv/custom/syntacore/common)

zephyr_sources(
  ../common/soc.c
)

# Common sources for the SCR5 V1 series

zephyr_include_directories_ifdef(CONFIG_CPU_SCR5_V1 ./v1)

# Common sources for the SCR5 V2 series

zephyr_include_directories_ifdef(CONFIG_CPU_SCR5_V2 ./v2)

# Common sources for the SCR5 V3 series

zephyr_include_directories_ifdef(CONFIG_CPU_SCR5_V3 ./v3)
