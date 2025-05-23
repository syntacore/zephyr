/*
 * Copyright (c) 2011-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * Game of life, based on https://github.com/andre-j3sus/game-of-life/tree/main
 *
 */

#include <zephyr/kernel.h>

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#else
#include <zephyr/sys/printk.h>
#endif

#include <zephyr/sys/__assert.h>

#include <zephyr/kernel_version.h>

#include "life.h"

void add_glider_a(int test_world[MAX_ROWS][MAX_COLS], int glider_r, int glider_c) 
{
	set_cell(test_world, glider_r + 0, glider_c + 0, 1);
	set_cell(test_world, glider_r + 0, glider_c + 1, 1);
	set_cell(test_world, glider_r + 0, glider_c + 2, 1);
	set_cell(test_world, glider_r + 1, glider_c + 2, 1);
	set_cell(test_world, glider_r + 2, glider_c + 1, 1);
}

void add_glider_b(int test_world[MAX_ROWS][MAX_COLS], int glider_r, int glider_c) 
{
	set_cell(test_world, glider_r + 0, glider_c + 0, 1);
	set_cell(test_world, glider_r + 1, glider_c + 0, 1);
	set_cell(test_world, glider_r + 2, glider_c + 0, 1);
	set_cell(test_world, glider_r + 2, glider_c + 1, 1);
	set_cell(test_world, glider_r + 1, glider_c + 2, 1);
}

int rule[RULE_SIZE] = {3, 2, 3}; // Default rule

int test_world[MAX_ROWS][MAX_COLS];
int aux_world[MAX_ROWS][MAX_COLS];

#define ESC_CLS "\x1b[2J"
#define ESC_GO_HOME "\x1b[H"
#define ESC_GO_HOME_3_0 "\x1b[3;0H"
#define ESC_INVISIBLE_CURSOR "\x1b[?25l"

int main(void)
{
	
	uint32_t zv = sys_kernel_version_get();

	printk(ESC_CLS);
	printk(ESC_GO_HOME);
	printk(ESC_INVISIBLE_CURSOR);
	printk("Game of life on Zephyr %d.%d.%d\n", SYS_KERNEL_VER_MAJOR(zv), SYS_KERNEL_VER_MINOR(zv), SYS_KERNEL_VER_PATCHLEVEL(zv));


	// Redo test_world
	clear_world(test_world, VIRTUAL_MAX_ROWS, VIRTUAL_MAX_COLS);
	
	//printk("Add gliders\n");
	printk("\n");

	add_glider_a(test_world, 1, 1);
	add_glider_b(test_world, 7, 7);

	uint32_t generation = 1;
	while(1)
	{

		printk(ESC_GO_HOME_3_0);
		printk("\n");
		print_world(test_world, 11, 11);
		printk("Generation: %d\n", generation); generation++;
		k_msleep(1000/12);

		update_world(test_world, 11, 11, aux_world, rule);

		
	}


#ifdef CONFIG_COVERAGE
	/* Wait a few seconds before main() exit, giving the sample the
	 * opportunity to dump some output before coverage data gets emitted
	 */
	k_sleep(K_MSEC(5000));
#endif
	return 0;
}
