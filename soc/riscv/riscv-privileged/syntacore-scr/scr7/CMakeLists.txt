# Copyright (c) 2021 Henrik Brix Andersen <henrik@brixandersen.dk>
# SPDX-License-Identifier: Apache-2.0

zephyr_sources(
  soc.c
  ../common/mpu.c
  ../common/cache.c
  ../common/plic.c
  ../common/iccm.c
)
