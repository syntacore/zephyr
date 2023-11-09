/*
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>

/******************************************************************************
 *                              Defines section
 */
#define BUFFER_SIZE 64
BUILD_ASSERT(BUFFER_SIZE >= sizeof(uint64_t), "Invalid buffer size");

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Misaligned access test procedure
 */
static void misaligned_test(void)
{
	uint8_t buffer[BUFFER_SIZE];

	for (uint32_t i = 0; i < ARRAY_SIZE(buffer); i++) {
		buffer[i] = i;
	}

	for (uint32_t i = 0; i + sizeof(uint64_t) < ARRAY_SIZE(buffer) + 1; i++) {
		uint64_t *ptr = (uint64_t*)&buffer[i];

		printk("Reading %x [%lx]:%llx\n", i, (uintptr_t)ptr, *ptr);
	}

	for (uint32_t i = 0; i + sizeof(uint64_t) < ARRAY_SIZE(buffer) + 1; i++) {
		uint64_t *ptr = (uint64_t*)&buffer[i];

		*ptr = i + 1;

		printk("Write/Reading %x [%lx]:%llx\n", i, (uintptr_t)ptr, *ptr);
	}

	for (uint32_t i = 0; i + sizeof(uint64_t) < ARRAY_SIZE(buffer) + 1; i++) {
		uint64_t *ptr = (uint64_t*)&buffer[i];

		printk("Reading %x [%lx]:%llx\n", i, (uintptr_t)ptr, *ptr);
	}
}

/**
 * @brief Main entry point
 *
 * @return 0
 */
int main(void)
{
	printk("Misaligned access test for %s\n", CONFIG_BOARD);
	misaligned_test();
	printk("Misaligned access test completed\n");

	return 0;
}
