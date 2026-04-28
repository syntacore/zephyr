/*
 * Copyright (c) 2024 MASSDRIVER EI (massdriver.space)
 * Copyright (c) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>

#include <zephyr/init.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <zephyr/spinlock.h>
#include <zephyr/irq.h>

#if DT_HAS_COMPAT_STATUS_OKAY(syntacore_machine_timer)
#define DT_DRV_COMPAT syntacore_machine_timer
#else
#define DT_DRV_COMPAT riscv_machine_timer
#endif

#if defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) ||			\
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
extern unsigned int z_clock_hw_cycles_per_sec;
#endif

#define MTIME_REG    DT_INST_REG_ADDR_BY_NAME(0, mtime)
#define MTIMECMP_REG DT_INST_REG_ADDR_BY_NAME(0, mtimecmp)
#define TIMER_IRQN   DT_INST_IRQN(0)

#define CYC_PER_TICK (uint32_t)(sys_clock_hw_cycles_per_sec() / CONFIG_SYS_CLOCK_TICKS_PER_SEC)

/* the unsigned long cast limits divisions to native CPU register width */
#define cycle_diff_t   unsigned long
#define CYCLE_DIFF_MAX (~(cycle_diff_t)0)

/*
 * We have two constraints on the maximum number of cycles we can wait for.
 *
 * 1) sys_clock_announce() accepts at most INT32_MAX ticks.
 *
 * 2) The number of cycles between two reports must fit in a cycle_diff_t
 *    variable before converting it to ticks.
 *
 * Then:
 *
 * 3) Pick the smallest between (1) and (2).
 *
 * 4) Take into account some room for the unavoidable IRQ servicing latency.
 *    Let's use 3/4 of the max range.
 *
 * Finally let's add the LSB value to the result so to clear out a bunch of
 * consecutive set bits coming from the original max values to produce a
 * nicer literal for assembly generation.
 */
#define CYCLES_MAX_1 ((uint64_t)INT32_MAX * (uint64_t)CYC_PER_TICK)
#define CYCLES_MAX_2 ((uint64_t)CYCLE_DIFF_MAX)
#define CYCLES_MAX_3 MIN(CYCLES_MAX_1, CYCLES_MAX_2)
#define CYCLES_MAX_4 (CYCLES_MAX_3 / 2 + CYCLES_MAX_3 / 4)
#define CYCLES_MAX   (CYCLES_MAX_4 + LSB_GET(CYCLES_MAX_4))

static struct k_spinlock lock;
static uint64_t last_count;
static uint64_t last_ticks;
static uint32_t last_elapsed;

#if defined(CONFIG_TEST)
const int32_t z_sys_timer_irq_for_test = TIMER_IRQN;
#endif

static inline uintptr_t get_hart_mtimecmp_addr(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(syntacore_machine_timer) && \
		!DT_HAS_COMPAT_STATUS_OKAY(sifive_clint0)
	return MTIMECMP_REG;
#else
	return MTIMECMP_REG + (arch_proc_id() * 8);
#endif
}

static inline uintptr_t get_mtime_addr(void)
{
	return MTIME_REG;
}

static uint64_t mtimer_get(uintptr_t reg)
{
#ifdef CONFIG_64BIT
	return *(volatile uint64_t *)reg;
#else
	volatile uint32_t *r = (uint32_t *)reg;
	uint32_t lo, hi;

	/* Likewise, must guard against rollover when reading */
	do {
		hi = r[1];
		lo = r[0];
	} while (r[1] != hi);

	return (((uint64_t)hi) << 32) | lo;
#endif
}

static void mtimer_set(uintptr_t reg, uint64_t value) {
#ifdef CONFIG_64BIT
	*(volatile uint64_t *)reg = value;
#else
	volatile uint32_t *r = (uint32_t *)reg;

	/* Per spec, the RISC-V MTIME/MTIMECMP registers are 64 bit,
	 * but are NOT internally latched for multiword transfers.  So
	 * we have to be careful about sequencing to avoid triggering
	 * spurious interrupts: always set the high word to a max
	 * value first.
	 */
	r[1] = 0xffffffff;
	r[0] = (uint32_t) value;
	r[1] = (uint32_t)(value >> 32);
#endif
}

#if defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
void z_sys_clock_hw_cycles_per_sec_update(uint32_t new_cycles)
{
	if (new_cycles == 0U) {
		return;
	}

	uint32_t old_cycles = (uint32_t)z_clock_hw_cycles_per_sec;

	if (old_cycles == new_cycles) {
		return;
	}

	k_spinlock_key_t key = k_spin_lock(&lock);

	uint64_t now = mtimer_get(get_mtime_addr());
	uint64_t next = mtimer_get(get_hart_mtimecmp_addr());
	uint64_t new_load;

	z_clock_hw_cycles_per_sec = new_cycles;

	/* Rescale internal counters from old cycles to new cycles. */
	last_count = last_count * (uint64_t) new_cycles / (uint64_t) old_cycles;

	/* Set new mtime with rescaled number of cycles*/
	mtimer_set(get_mtime_addr(), now * (uint64_t) new_cycles / (uint64_t) old_cycles);

	/* Set new mtimecmp with rescaled number of cycles*/
	if (IS_ENABLED(CONFIG_TICKLESS_KERNEL)) {
		new_load = (next - now) * (uint64_t) new_cycles / (uint64_t) old_cycles;
	} else {
		new_load = CYC_PER_TICK;
	}

	mtimer_set(get_hart_mtimecmp_addr(), new_load);

	k_spin_unlock(&lock, key);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

static void timer_isr(const void *arg)
{
	ARG_UNUSED(arg);

	k_spinlock_key_t key = k_spin_lock(&lock);

	uint64_t now = mtimer_get(get_mtime_addr());
	uint64_t dcycles = now - last_count;
	uint32_t dticks = (cycle_diff_t)dcycles / CYC_PER_TICK;

	last_count += (cycle_diff_t)dticks * CYC_PER_TICK;
	last_ticks += dticks;
	last_elapsed = 0;

	if (!IS_ENABLED(CONFIG_TICKLESS_KERNEL)) {
		uint64_t next = last_count + CYC_PER_TICK;

		mtimer_set(get_hart_mtimecmp_addr(), next);
	}

	k_spin_unlock(&lock, key);
	sys_clock_announce(dticks);
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
	ARG_UNUSED(idle);

	if (!IS_ENABLED(CONFIG_TICKLESS_KERNEL)) {
		return;
	}

	k_spinlock_key_t key = k_spin_lock(&lock);
	uint64_t cyc;

	if (ticks == K_TICKS_FOREVER) {
		cyc = last_count + CYCLES_MAX;
	} else {
		cyc = (last_ticks + last_elapsed + ticks) * CYC_PER_TICK;
		if ((cyc - last_count) > CYCLES_MAX) {
			cyc = last_count + CYCLES_MAX;
		}
	}
	mtimer_set(get_hart_mtimecmp_addr(), cyc);

	k_spin_unlock(&lock, key);
}

uint32_t sys_clock_elapsed(void)
{
	if (!IS_ENABLED(CONFIG_TICKLESS_KERNEL)) {
		return 0;
	}

	k_spinlock_key_t key = k_spin_lock(&lock);
	uint64_t now = mtimer_get(get_mtime_addr());
	uint64_t dcycles = now - last_count;
	uint32_t dticks = (cycle_diff_t)dcycles / CYC_PER_TICK;

	last_elapsed = dticks;
	k_spin_unlock(&lock, key);
	return dticks;
}

uint32_t sys_clock_cycle_get_32(void)
{
	return ((uint32_t)mtimer_get(get_mtime_addr())) << CONFIG_RISCV_MACHINE_TIMER_SYSTEM_CLOCK_DIVIDER;
}

uint64_t sys_clock_cycle_get_64(void)
{
	return mtimer_get(get_mtime_addr()) << CONFIG_RISCV_MACHINE_TIMER_SYSTEM_CLOCK_DIVIDER;
}

static int sys_clock_driver_init(void)
{
	IRQ_CONNECT(TIMER_IRQN, 0, timer_isr, NULL, 0);
	last_ticks = mtimer_get(get_mtime_addr()) / CYC_PER_TICK;
	last_count = last_ticks * CYC_PER_TICK;

	mtimer_set(get_hart_mtimecmp_addr(), last_count + CYC_PER_TICK);
	irq_enable(TIMER_IRQN);
	return 0;
}

#ifdef CONFIG_SMP
void smp_timer_init(void)
{
	mtimer_set(get_hart_mtimecmp_addr(), last_count + CYC_PER_TICK);
	irq_enable(TIMER_IRQN);
}
#endif

SYS_INIT(sys_clock_driver_init, PRE_KERNEL_2, CONFIG_SYSTEM_CLOCK_INIT_PRIORITY);
