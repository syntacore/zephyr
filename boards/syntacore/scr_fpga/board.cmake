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

set(SUPPORTED_EMU_PLATFORMS qemu)

if(NOT (CONFIG_SOC_SCR1))
  set(QEMU_binary_suffix riscv64)
  set(QEMU_CPU_TYPE_${ARCH} riscv64)
  
  if(CONFIG_SOC_SCR7_L2)
    set(QEMU_FLAGS_${ARCH}
      -nographic
      -machine scr7_l2
    )
    
    board_runner_args(openocd  --use-elf "--config=${BOARD_DIR}/support/syntacore_riscv_quad.cfg")
  elseif(CONFIG_SOC_SCR9_L2)
    set(QEMU_FLAGS_${ARCH}
      -nographic
      -machine scr9_l2
    )
    
    board_runner_args(openocd  --use-elf "--config=${BOARD_DIR}/support/syntacore_riscv.cfg")
  elseif(CONFIG_SOC_SCR9_L3)
    set(QEMU_FLAGS_${ARCH}
      -nographic
      -machine scr9_l3
    )
    
    board_runner_args(openocd  --use-elf "--config=${BOARD_DIR}/support/syntacore_riscv_dual.cfg")
  endif()
else()
  set(QEMU_binary_suffix riscv32)
  set(QEMU_CPU_TYPE_${ARCH} riscv32)

  set(QEMU_FLAGS_${ARCH}
    -nographic
    -machine scr1,ipic=on
  )
  
  board_runner_args(openocd  --use-elf "--config=${BOARD_DIR}/support/syntacore_riscv.cfg")
endif()


board_set_debugger_ifnset(qemu)

set(OPENOCD_USE_LOAD_IMAGE YES)

board_runner_args(openocd  "--config=digilent.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
