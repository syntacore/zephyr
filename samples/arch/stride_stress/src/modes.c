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
/*
 * @file modes.c
 *
 * @author vdsh-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include "modes.h"
#include <zephyr/sys/crc.h>

/******************************************************************************
 *                              Prototypes section
 */

static void calc_crc(uint32_t* crc_result, struct region_crc* regions, int region_nums)
{
    for (int j = 0; j < region_nums; j++) {
        crc_result[j] = crc32_ieee_update(0, regions[j].ptr, regions[j].size);
    }
}

/**
 * @brief Reads from each memory location of size CACHE_LINE from range
 */
void cache_lines_clean(struct mem_descriptor* descriptor, size_t* hash_errors)
{
    size_t iterations = ITERATIONS;                 /* +1 becaues of count > 0 */
    size_t count = descriptor->range / (descriptor->stride * HUNDRED_TIMES) + 1;
    size_t i;
    char** volatile p = (char** volatile) descriptor->start_ptr;

    /* regions need to check memory integrity by crc. You can add more regions */

    struct region_crc regions[] = {
        {(const uint8_t *) descriptor->raw_ptr, sizeof(struct mem_raw)}
    };

    if (IS_ENABLED(CONFIG_DEBUG)) {
        for (int j = 0; j < ARRAY_SIZE(regions); j++) {
            printk("crc%d: start - %p, size - 0x%lx, end - %p\n", 
                j, regions[j].ptr, regions[j].size, regions[j].ptr + regions[j].size);
        }
    }

    uint32_t crc_before[ARRAY_SIZE(regions)];
    calc_crc(crc_before, regions, ARRAY_SIZE(regions));

    while (iterations-- > 0) {
        for (i = 0; i < count; ++i) {
            HUNDRED;        
        }
    }

    uint32_t crc_after[ARRAY_SIZE(regions)];
    calc_crc(crc_after, regions, ARRAY_SIZE(regions));

    if (IS_ENABLED(CONFIG_DEBUG)) {
        for (int j = 0; j < ARRAY_SIZE(regions); j++) {
            printk("[%d]: before - %d, after - %d, result: %u\n", 
                j, crc_before[j], crc_after[j], crc_before[j] == crc_after[j]);
        }
    }
    for (int j = 0; j < ARRAY_SIZE(regions); j++) {
        *hash_errors += (crc_before[j] != crc_after[j]);
    }
}

/**
 * @brief Pollute each base_stride_struct once
 */
static void pollute_arr_once(size_t poison, struct dirty_stride_struct* array_itr, size_t count) {

    for (int i = 0; i < count; ++i) {
        array_itr->poisoned = poison;
        array_itr = (struct dirty_stride_struct*) array_itr->next;
    }
}

/**
 * @brief We pollute the array first with a new value, and then pollute the previous 
 *        one - and the total hash from the array does not change
 */
static void touch_arr(size_t poison, struct dirty_stride_struct* array_itr, size_t count) {

    poison++;
    pollute_arr_once(poison, array_itr, count);
    poison--;
    pollute_arr_once(poison, array_itr, count);
}

/**
 * @brief Reads and writes to each memory location of size CACHE_LINE from range
 */
void cache_lines_dirty(struct mem_descriptor* descriptor, size_t* hash_errors)
{
    size_t iterations = descriptor->iterations;
    size_t count = NUM_ENTRIES;

    /* regions need to check memory integrity by crc. You can add more regions */

    struct region_crc regions[] = {
        {(const uint8_t *) descriptor->raw_ptr, sizeof(struct mem_raw)}
    };

    if (IS_ENABLED(CONFIG_DEBUG)) {
        for (int j = 0; j < ARRAY_SIZE(regions); j++) {
            printk("crc%d: start - %p, size - 0x%lx, end - %p\n", 
                j, regions[j].ptr, regions[j].size, regions[j].ptr + regions[j].size);
        }
    }

    size_t poison = _current->stack_info.start;

    uint32_t crc_before[ARRAY_SIZE(regions)];
    uint32_t crc_after[ARRAY_SIZE(regions)];

    struct dirty_stride_struct* array_itr = (struct dirty_stride_struct*) descriptor->start_ptr;

    for (; iterations != 0; iterations--) {
        k_mutex_lock(descriptor->mutex, K_FOREVER);
        touch_arr(poison, array_itr, count);

        /* On first iteration we calculate hash summ [after initialization!] */
        if (iterations == descriptor->iterations) {
            calc_crc(crc_before, regions, ARRAY_SIZE(regions));
        }

        /* On last iteration we calculate hash summ [before mutex unlock!] */
        if (iterations == 1) {
            calc_crc(crc_after, regions, ARRAY_SIZE(regions));
        }

        k_mutex_unlock(descriptor->mutex);
    }

    if (IS_ENABLED(CONFIG_DEBUG)) {
        for (int j = 0; j < ARRAY_SIZE(regions); j++) {
            printk("[%d]: before - %d, after - %d, result: %u\n", 
                j, crc_before[j], crc_after[j], crc_before[j] == crc_after[j]);
        }
    }
    for (int j = 0; j < ARRAY_SIZE(regions); j++) {
        *hash_errors += (crc_before[j] != crc_after[j]);
    }
}

void thread_routine(struct mem_descriptor* descriptor, size_t* hash_errors)
{
    uint64_t start_time, stop_time, cycles_spent, milliseconds_spent;
    size_t iterations = ITERATIONS;
    size_t count = NUM_ENTRIES;
    size_t range = descriptor->range;

    if (IS_ENABLED(CONFIG_DEBUG)) {
        printk("Now is %ld cache line max used\n", range / CACHE_LINE);
        printk("stack: start - %p, size - 0x%lx, end - %p\n", 
            (char*) _current->stack_info.start, _current->stack_info.size, 
            (char*) _current->stack_info.start +  _current->stack_info.size);
        printk("array: start - %p, size - 0x%lx, end - %p\n", 
            descriptor->start_ptr, range, descriptor->start_ptr + range);
        printk("Count: %ld, iterations - %ld\n", count, iterations);
    }

    start_time = k_cycle_get_64();
    
    (descriptor->action)(descriptor, hash_errors);

    stop_time = k_cycle_get_64();

    cycles_spent = stop_time - start_time;
    milliseconds_spent = (uint64_t) k_cyc_to_ms_floor64(cycles_spent);

    printk("FINISHED in %lld msec\n", milliseconds_spent);
}
