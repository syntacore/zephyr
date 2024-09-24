/*
 * Copyright (C) 2023, Syntacore Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
