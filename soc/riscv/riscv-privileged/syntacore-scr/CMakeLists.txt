# SPDX-License-Identifier: Apache-2.0

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(../common ./common)
add_subdirectory(${SOC_SERIES})

zephyr_include_directories(./common)

zephyr_sources_ifdef(CONFIG_SCR_SYSTEM_HALT ./common/halt.c)