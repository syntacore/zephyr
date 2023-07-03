/*
 * @file main.c
 *
 * @brief Syntacore Naive Membarrier test
 *
 * @author vdsh-sc
 *
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
 *                              Defines section
 */
#define STACK_SIZE 2048 
#define THREADS_NUM 8

#define RISCV_FENCE(p, s) \
    __asm__ __volatile__ ("fence " #p "," #s : : : "memory")

/* These barriers need to enforce ordering on both devices or memory. */
#define mb()  RISCV_FENCE(rw,rw)
#define rmb() RISCV_FENCE(r,r)
#define wmb() RISCV_FENCE(w,w)

#define PRIORITY 10

#define RATELIMIT (ITERATIONS_COUNT / 100000)
#define ITERATIONS_COUNT (1000000000000LLU)

/******************************************************************************
 *                              Static variables section
 */
static K_THREAD_STACK_ARRAY_DEFINE(tstack, THREADS_NUM, STACK_SIZE);
static k_tid_t tinfo[THREADS_NUM];
static struct k_thread tthread[THREADS_NUM];
static int read_write_buf[2];
static int global;

/******************************************************************************
 *                              Prototypes section
 */

/**
 * @brief Routine with membarrier
 *
 * @param [in] arg1 - first  ptr to read_write_buf
 * @param [in] arg2 - second ptr to read_write_buf
 * @param [in] arg3 - unused
 */

static void routine(void *arg1, void *arg2, void *arg3)
{
    int *x = arg1;
    int *y = arg2;

    ARG_UNUSED(arg3);

    printk("thread start\n");

    k_tid_t curr_tid = k_current_get();

    for (unsigned long long i = 0; i < ITERATIONS_COUNT; i++) 
    {
        *x = global;
        
        if (i % 3 == 0)
            mb();
        else if (i % 3 == 1)
            wmb();
        else 
            rmb();
        
        global = *y;
        
        if ((i % RATELIMIT) == 0) 
        {
            printk("%lld: %lld more times passed, base ptr - %p\n", i, RATELIMIT, &(curr_tid->base));
        }
    }

    printk("thread finish\n");
}

/**
 * @brief threads info init
 */

static void threads_init(void)
{
    memset(tthread, 0x00, sizeof(tthread));
    memset(tinfo, 0x00, sizeof(tinfo));
    memset(tstack, 0x00, sizeof(tstack));
}

void main()
{
    threads_init();

    for (int i = 0; i < THREADS_NUM; i++) {

        tinfo[i] = k_thread_create(&tthread[i], tstack[i], STACK_SIZE,
                    (k_thread_entry_t)routine,
                    (void *) (read_write_buf + (i & 1)), (void *) (read_write_buf + 1 - (i & 1)), NULL,
                    K_PRIO_PREEMPT(PRIORITY), 0, K_NO_WAIT);
    }

    printk("All thread created\n");

    for (int i = 0; i < THREADS_NUM; i++) {

        k_thread_join(tinfo[i], K_FOREVER);
    }

    printk("TEST PASSED\n");
}
