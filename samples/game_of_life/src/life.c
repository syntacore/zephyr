/*
 * Copyright (C) 2025, Syntacore Ltd.
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

#include <zephyr/kernel.h>

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#else
#include <zephyr/sys/printk.h>
#endif

#include <zephyr/sys/__assert.h>

#include "life.h"

int cell_lives(const int submatrix[3][3], const int rule[RULE_SIZE])
{
	int cell = submatrix[1][1];
	int live_cells = 0;

	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			if (col == 1 && row == 1)
				continue;

			live_cells += submatrix[row][col];
		}
	}

	return (cell == 1 && (live_cells >= rule[1] && live_cells <= rule[0])) ||
		   (cell == 0 && live_cells == rule[2]);
}

void clear_world(int world[][MAX_COLS], int rows_count, int cols_count)
{
	if (cols_count > VIRTUAL_MAX_COLS || rows_count < 2 || cols_count < 2)
		return;

	for (size_t row = 1; row < rows_count + 1; row++)
	{
		for (size_t col = 1; col < cols_count + 1; col++)
		{
			world[row][col] = 0;
		}
	}
}

void set_cell(int world[][MAX_COLS], int row, int col, int value)
{
	world[row][col] = value;
}

int get_cell(const int world[][MAX_COLS], int row, int col)
{
	return world[row][col];
}

void copy_world(
	int world1[][MAX_COLS], int rows_count, int cols_count,
	int world2[][MAX_COLS])
{
	for (size_t row = 1; row < rows_count + 1; row++)
	{
		for (size_t col = 1; col < cols_count + 1; col++)
		{
			world1[row][col] = world2[row][col];
		}
	}
}

void update_world(
	int world[][MAX_COLS],
	int rows_count, int cols_count,
	int world_aux[][MAX_COLS], const int rule[RULE_SIZE])
{
	for (size_t row = 1; row < rows_count + 1; row++)
	{
		for (size_t col = 1; col < cols_count + 1; col++)
		{
			int pre_row = row - 1;
			if (pre_row == 0)
				pre_row = rows_count;

			int pos_row = row + 1;
			if (pos_row == rows_count + 1)
				pos_row = 1;

			int pre_col = col - 1;
			if (pre_col == 0)
				pre_col = cols_count;

			int pos_col = col + 1;
			if (pos_col == cols_count + 1)
				pos_col = 1;

			int submatrix[3][3];
			submatrix[0][0] = get_cell(world, pre_row, pre_col);
			submatrix[0][1] = get_cell(world, pre_row, col);
			submatrix[0][2] = get_cell(world, pre_row, pos_col);
			submatrix[1][0] = get_cell(world, row, pre_col);
			submatrix[1][1] = get_cell(world, row, col);
			submatrix[1][2] = get_cell(world, row, pos_col);
			submatrix[2][0] = get_cell(world, pos_row, pre_col);
			submatrix[2][1] = get_cell(world, pos_row, col);
			submatrix[2][2] = get_cell(world, pos_row, pos_col);

			set_cell(world_aux, row, col, cell_lives(submatrix, rule));
		}
	}

	copy_world(world, rows_count, cols_count, world_aux);
}

void update_world_n_generations(
	int n, int world[][MAX_COLS],
	int rows_count, int cols_count,
	int world_aux[][MAX_COLS],
	const int rule[RULE_SIZE])
{
	if (n <= 0)
		return;

	for (size_t i = 0; i <= n; i++)
	{
		update_world(world, rows_count, cols_count, world_aux, rule);
	}
}
#if COLOR_PRINTS
void print_world(const int world[][MAX_COLS], int rows_count, int cols_count)
{
	static int color_idx = 0;
	static const int color_idx_max = 8;
	//static char life_syms[] = {'*', 'x', 'X', 'O', 'o', 'Y', 'H', 'T', 'Z', 'z', 'c', 'C', 'v', 'V', 'N', 'w', 'W', 'M'};
	static char life_syms[] = {'*', 'x', 'X', 'O', 'o', '.'};
	static int life_syms_idx = 0;
	static const int life_syms_idx_max = (int)(sizeof(life_syms) / sizeof(life_syms[0]));
	for (size_t row = 1; row < rows_count + 1; row++)
	{
		for (size_t col = 1; col < cols_count + 1; col++)
		{
			if(world[row][col])
			{
				printk("\x1b[1;%dm%c ", color_idx + 30, life_syms[life_syms_idx]);
				life_syms_idx = (life_syms_idx + 1) % life_syms_idx_max;
				color_idx = (color_idx + 1) % color_idx_max;
			}
			else printk("\x1b[0m  ");
		}
		printk("\x1b[0m\n");
	}
}
#else 
void print_world(const int world[][MAX_COLS], int rows_count, int cols_count)
{
	for (size_t row = 1; row < rows_count + 1; row++)
	{
		for (size_t col = 1; col < cols_count + 1; col++)
		{
			//putchar((world[row][col]) ? "X " : ". ");
			// putchar((world[row][col]) ? 'X' : '.');
			// putchar(' ');
			printk("%c ", (world[row][col]) ? 'X' : '.');
		}
		printk("\n");
	}
}
#endif 
