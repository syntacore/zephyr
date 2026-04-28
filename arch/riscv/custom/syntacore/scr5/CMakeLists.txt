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

# Common sources for all SCR5 cores 

zephyr_include_directories(../common)

zephyr_sources(
  ../common/mtimer.c
  ../common/plic.c
  ../common/cache.c
)

zephyr_sources_ifdef(CONFIG_SCR_SYSTEM_HALT ../common/halt.c)
zephyr_sources_ifdef(CONFIG_SCR_MPU ../common/mpu.c)

zephyr_sources_ifndef(CONFIG_CPU_SCR5_V3
  ../common/iccm.c
  ../common/ipi_iccm.c
)
