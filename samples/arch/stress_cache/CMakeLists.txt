# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stress_cache)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/cache.c)
target_sources(app PRIVATE src/mwc.c)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/soc/riscv/riscv-privileged/syntacore-scr/common)
