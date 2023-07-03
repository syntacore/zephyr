# SPDX-License-Identifier: Apache-2.0

set(SUPPORTED_EMU_PLATFORMS qemu)

set(QEMU_binary_suffix riscv32)
set(QEMU_CPU_TYPE_${ARCH} riscv32)

set(QEMU_FLAGS_${ARCH}
  -nographic
  -machine syntacore_scr1,ipic=on
  )

board_set_debugger_ifnset(qemu)

set(OPENOCD_USE_LOAD_IMAGE YES)

board_runner_args(openocd  --use-elf "--config=${BOARD_DIR}/support/scr1.cfg")
board_runner_args(openocd  "--config=digilent.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
