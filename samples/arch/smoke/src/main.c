/*
 * @file main.c
 *
 * @brief Syntacore Smoke Test Utility
 *
 * @author amr-sc
 *
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/console/console.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
 *                              Defines section
 */
/* Amount of Pi calculating threads to create and run */
#define THREADS_NUM	16

/*
 * Amount of digits of Pi to calculate, must be a multiple of 4,
 * as used algorithm spits 4 digits on every iteration.
 */
#if defined(CONFIG_BOARD_VCU118_SCR1)
/* For SCR1 make less iterations of calculating pi
 * due to RAM limitations.
 */
#define DIGITS_NUM	92
#else
#define DIGITS_NUM	240
#endif

#define LENGTH		((DIGITS_NUM / 4) * 14)
#define STACK_SIZE	((LENGTH * sizeof(int) + 1024))

#ifdef CONFIG_SMP
#define CORES_NUM	CONFIG_MP_NUM_CPUS
#else
#define CORES_NUM	1
#endif

#define NEW_BASE	10000
#define ARRAY_INIT	2000

#define TIMER_TEST_DURATION_SEC 10

/******************************************************************************
 *                              Prototypes section
 */
static void on_timer_expiration(struct k_timer *timer);
static void on_timer_expiration_work(struct k_work *work);

/******************************************************************************
 *                              Static variables section
 */
static K_TIMER_DEFINE(my_timer, on_timer_expiration, NULL);
static K_WORK_DEFINE(my_work, on_timer_expiration_work);

static uint32_t seconds;
static K_THREAD_STACK_ARRAY_DEFINE(tstack, THREADS_NUM, STACK_SIZE);
static struct k_thread tthread[THREADS_NUM];
static char buffer[THREADS_NUM][DIGITS_NUM + 1];
static atomic_t counter = THREADS_NUM;

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Timer expiration work
 *
 * @param [in] work - Zephyr work
 */
static void on_timer_expiration_work(struct k_work *work)
{
	printk("Seconds passed %d\n", seconds);

	ARG_UNUSED(work);
}

/**
 * @brief Timer expiration handler
 *
 * @param [in] timer - timer object
 */
static void on_timer_expiration(struct k_timer *timer)
{
	if (seconds >= TIMER_TEST_DURATION_SEC) {
		k_timer_stop(&my_timer);
		seconds = 0;
		return;
	}

	seconds++;
	k_work_submit(&my_work);

	ARG_UNUSED(timer);
}

/**
 * @brief Pi calculation thread
 *
 * @param [in] arg1 - thread argument 1
 * @param [in] arg2 - thread argument 2
 * @param [in] arg3 - thread argument 3
 */
static void test_thread(void *arg1, void *arg2, void *arg3)
{
	atomic_t *counter = (atomic_t *)arg1;
	char *buffer = (char *)arg2;

	ARG_UNUSED(arg3);

	int array[LENGTH + 1] = {};
	int carry = 0;
	int i, j;

	for (i = 0; i < LENGTH; i++) {
		array[i] = ARRAY_INIT;
	}

	for (i = LENGTH; i > 0; i -= 14) {
		int sum = 0, value;

		for (j = i; j > 0; --j) {
			sum = sum * j + NEW_BASE * array[j];
			array[j] = sum % (j * 2 - 1);
			sum /= j * 2 - 1;
		}

		value = carry + sum / NEW_BASE;
		carry = sum % NEW_BASE;

		/* Convert 4-digit int to string */
		sprintf(buffer, "%.4d", value);
		buffer += 4;
	}

	atomic_dec(counter);
}

/**
 * @brief SMP test initialization
 */
static void smp_test_init(void)
{
	memset(tthread, 0x00, sizeof(tthread));
	memset(tstack, 0x00, sizeof(tstack));
	memset(buffer, 0x00, sizeof(buffer));
	atomic_set(&counter, THREADS_NUM);
}

/**
 * @brief SMP test calculating PI using multiple threads
 */
static void smp_test(void)
{
	uint32_t start_time, stop_time, cycles_spent, nanoseconds_spent;
	int i;

	smp_test_init();

	/* Capture initial time stamp */
	start_time = k_cycle_get_32();

	for (i = 0; i < THREADS_NUM; i++) {
		k_thread_create(&tthread[i], tstack[i], STACK_SIZE,
			       (k_thread_entry_t)test_thread,
			       (void *)&counter, (void *)buffer[i], NULL,
			       K_PRIO_COOP(10), 0, K_NO_WAIT);
	}

	/* Wait for all workers to finish their calculations */
	while (counter) {
		k_sleep(K_MSEC(1));
	}

	/* Capture final time stamp */
	stop_time = k_cycle_get_32();

	cycles_spent = stop_time - start_time;
	nanoseconds_spent = (uint32_t)k_cyc_to_ns_floor64(cycles_spent);

	printk(" All %d threads executed by %d cores in %d msec\n", THREADS_NUM,
	       CORES_NUM, nanoseconds_spent / 1000 / 1000);
	printk("SMP test completed\n");
}

/**
 * @brief MTIMER test
 */
static void timer_test(void)
{
	printk("Timer test begins\n");
	k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));
}

/**
 * @brief Smoke Test Utility entry point
 */
void main(void)
{
	console_getline_init();

	printk("Syntacore Smoke Tests Utility\n");

	while (42) {
		char *s = console_getline();

		if (strcmp(s, "MTIMER") == 0) {
			timer_test();
		} else if (strcmp(s, "SMP") == 0) {
			smp_test();
		}
	}
}
