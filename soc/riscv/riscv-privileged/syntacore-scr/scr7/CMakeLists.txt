# Copyright (c) 2023 Syntacore
# SPDX-License-Identifier: Apache-2.0

zephyr_sources(
  soc.c
  ../common/cache.c
  ../common/plic.c
  ../common/iccm.c
  ../common/memcheck.cpp
)

zephyr_sources_ifdef(CONFIG_RISCV_PMP ../common/scr_pmp.c)
zephyr_sources_ifdef(CONFIG_SCR_MMU ../common/mmu.c)