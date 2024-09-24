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
 * @file main.c
 *
 * @author vdsh-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include "modes.h"
#include <string.h>

/******************************************************************************
 *                              Static variables section
 */
static K_THREAD_STACK_ARRAY_DEFINE(tstack, NUM_THREADS, STACK_SIZE);
static k_tid_t tinfo[NUM_THREADS];
static struct k_thread tthread[NUM_THREADS];

static __aligned(sizeof(char*)) struct mem_raw test_array;
static struct k_mutex mutex_for_descr;
static size_t hash_errors[NUM_THREADS];
static size_t total_hash_errors;

/******************************************************************************
 *                              Prototypes section
 */
static void threads_init(void)
{
    memset(tthread, 0x00, sizeof(tthread));
    memset(tinfo, 0x00, sizeof(tinfo));
    memset(tstack, 0x00, sizeof(tstack));
}

/**
 * @brief Stride memory initialization
 */
static void stride_initialize(struct mem_descriptor* descriptor)
{   
    if (IS_ENABLED(CONFIG_DEBUG)) {
        printk("start: %p, end: %p\n", descriptor->raw_ptr, descriptor->raw_ptr + sizeof(struct mem_raw));
        printk("array start: %p, array size - 0x%lx\n", descriptor->start_ptr, descriptor->max_range);
    }
    size_t i;

    struct base_stride_struct* addr = (struct base_stride_struct*) descriptor->start_ptr;
    size_t stride = descriptor->stride;
    size_t range = descriptor->range;

    for (i = 1; i < range / stride; i++) {
        addr[i - 1].next = &addr[i];
    }

    addr[range / stride - 1].next = &addr[0];
}

/**
 * @brief Runs benchmark threads
 */
static void run_benchmark(struct mem_descriptor* descriptor)
{
    for (int i = 0; i < NUM_THREADS; i++) {
        tinfo[i] = k_thread_create(&tthread[i], tstack[i], STACK_SIZE, 
                        (k_thread_entry_t) thread_routine,
                        (void *) descriptor, &hash_errors[i], NULL,
                        K_PRIO_PREEMPT(10), 0, K_NO_WAIT);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        k_thread_join(tinfo[i], K_FOREVER);     
    }
}

/**
 * @brief Gives the next range (from lmbench)
 */
static size_t get_next_range(size_t k)
{
    if (k < 1024) {
        k = k * 2;
    } else if (k < 4*1024) {
        k += 1024;
    } else {
        size_t s;

        for (s = 32 * 1024; s <= k; s *= 2)
            ;
        k += s / 16;
    }
    return (k);
}

static void run_test(size_t max_range, size_t range, size_t stride, 
                    void (*action) (struct mem_descriptor* descriptor, size_t* hash_errors))
{
    struct mem_descriptor descriptor;

    if (range < stride) {
        return;
    }
    if (range > max_range) {
        return;
    }

    descriptor.range  = range;
    descriptor.max_range = max_range;
    descriptor.stride = stride;
    descriptor.mutex = &mutex_for_descr;
    descriptor.action = action;
    descriptor.raw_ptr = test_array.left_canary[0];
    descriptor.start_ptr = test_array.main_array;
    descriptor.iterations = ITERATIONS;

    if (IS_ENABLED(CONFIG_DEBUG)) {
        printk("max_range - %ld, range - %ld, stride - %ld\n", max_range, range, stride);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        hash_errors[i] = 0;
    }

    stride_initialize(&descriptor);
    run_benchmark(&descriptor);

    for (int i = 0; i < NUM_THREADS; i++) {
        total_hash_errors += hash_errors[i];
    }
}

int main(int ac, char **av)
{
    threads_init();

    k_mutex_init(&mutex_for_descr);

    size_t range;
    for (range = LOWER; range <= MAX_RANGE; range = get_next_range(range)) {
        printk("\nStart CLEAN with %ld range\n", range);
        run_test(MAX_RANGE, range, STRIDE, cache_lines_clean);
    }

    for (range = LOWER; range <= MAX_RANGE; range = get_next_range(range)) {
        printk("\nStart DIRTY with %ld range\n", range);
        run_test(MAX_RANGE, range, STRIDE, cache_lines_dirty);
    }

    if (IS_ENABLED(CONFIG_DEBUG)) {
        printk("Size of max_num_entries: %d, size of num_entries: %d\n", 
                                        MAX_RANGE / CACHE_LINE, NUM_ENTRIES);
    }
    
    if (total_hash_errors == 0) {
        printk("\nTEST PASSED, NO HASH ERRORS\n");
    } else {
        printk("\nTOTAL: %ld HASH ERRORS\n", total_hash_errors);
    }

    return 0;
}
