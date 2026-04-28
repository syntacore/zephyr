/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/irq.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/arch/riscv/irq.h>
#include <zephyr/drivers/pm_cpu_ops.h>
#include <zephyr/platform/hooks.h>
#include <zephyr/cache.h>

volatile struct {
	arch_cpustart_t fn;
	void *arg;
} riscv_cpu_init[CONFIG_MP_MAX_NUM_CPUS];

volatile uintptr_t riscv_cpu_wake_flag __aligned(CONFIG_RISCV_MIN_STACK_OBJ_ALIGN_AND_SIZE) = -1;
volatile uintptr_t riscv_cpu_boot_flag __aligned(CONFIG_RISCV_MIN_STACK_OBJ_ALIGN_AND_SIZE) = -1;
volatile void *riscv_cpu_sp;
#ifdef CONFIG_STACK_CANARIES
extern volatile uintptr_t __stack_chk_guard;
#endif /* CONFIG_STACK_CANARIES */

extern void __start(void);
#ifdef CONFIG_RISCV_EARLY_INIT_SECONDARY
uint8_t riscv_cpu_node_list[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(cpus), DT_REG_ADDR, (,))
};
uint8_t riscv_cpu_node_list_size =
	MIN(ARRAY_SIZE(riscv_cpu_node_list), CONFIG_MP_MAX_NUM_CPUS);

#ifdef CONFIG_INIT_STACKS
K_KERNEL_PINNED_STACK_ARRAY_DEFINE(riscv_early_secondary_stacks,
				   CONFIG_MP_MAX_NUM_CPUS,
				   CONFIG_RISCV_EARLY_INIT_SECONDARY_STACK_SIZE);
#endif /* CONFIG_INIT_STACKS */

#endif /* CONFIG_RISCV_EARLY_INIT_SECONDARY */

#if defined(CONFIG_RISCV_SOC_INTERRUPT_INIT)
void soc_interrupt_init(void);
#endif

void arch_cpu_start(int cpu_num, k_thread_stack_t *stack, int sz,
		    arch_cpustart_t fn, void *arg)
{
	riscv_cpu_init[cpu_num].fn = fn;
	riscv_cpu_init[cpu_num].arg = arg;

	riscv_cpu_sp = K_KERNEL_STACK_BUFFER(stack) + sz;
	riscv_cpu_boot_flag = 0U;

#ifdef CONFIG_PM_CPU_OPS
	if (pm_cpu_on(cpu_num, (uintptr_t)&__start)) {
		printk("Failed to boot secondary CPU %d\n", cpu_num);
		return;
	}
#endif
#ifdef CONFIG_CACHE_MANAGEMENT
	/*
	 * Flush hartid, boot flag, stack pointer and stack canary value,
	 * since Secondary core works without caches still
	 */
	cache_data_flush_range((void *)&_kernel.cpus[cpu_num].arch.hartid,
		sizeof(_kernel.cpus[cpu_num].arch.hartid));
	cache_data_flush_range((void *)&riscv_cpu_sp, sizeof(riscv_cpu_sp));
	cache_data_flush_range((void *)&riscv_cpu_boot_flag,
		sizeof(riscv_cpu_boot_flag));
#ifdef CONFIG_STACK_CANARIES
	cache_data_flush_range((void *)&__stack_chk_guard,
		sizeof(__stack_chk_guard));
#endif /* CONFIG_STACK_CANARIES */
#ifdef CONFIG_INIT_STACKS
	cache_data_flush_range((void *)&z_interrupt_stacks,
		sizeof(z_interrupt_stacks));
#endif /* CONFIG_INIT_STACKS */
#endif /* CONFIG_CACHE_MANAGEMENT */

	while (riscv_cpu_boot_flag == 0U) {
		riscv_cpu_wake_flag = _kernel.cpus[cpu_num].arch.hartid;
#ifdef CONFIG_CACHE_MANAGEMENT
		cache_data_flush_range((void *)&riscv_cpu_wake_flag,
			sizeof(riscv_cpu_wake_flag));
		cache_data_invd_range((void *)&riscv_cpu_boot_flag,
			sizeof(riscv_cpu_boot_flag));
#endif /* CONFIG_CACHE_MANAGEMENT */
	}
}

void __weak riscv_init_secondary(void)
{
}

void arch_secondary_cpu_init(int hartid)
{
	unsigned int i;
	unsigned int cpu_num = 0;

	for (i = 0; i < CONFIG_MP_MAX_NUM_CPUS; i++) {
		if (_kernel.cpus[i].arch.hartid == hartid) {
			cpu_num = i;
		}
	}
	csr_write(mscratch, &_kernel.cpus[cpu_num]);
#if defined(CONFIG_MULTITHREADING) && defined(CONFIG_THREAD_LOCAL_STORAGE)
	__asm__("mv tp, %0" : : "r" (z_idle_threads[cpu_num].tls));
#endif
#if defined(CONFIG_RISCV_SOC_INTERRUPT_INIT)
	soc_interrupt_init();
#endif
	riscv_init_secondary();
#ifdef CONFIG_RISCV_PMP
	z_riscv_pmp_init();
#endif
#ifdef CONFIG_SMP
	irq_enable(RISCV_IRQ_MSOFT);
#endif /* CONFIG_SMP */
#ifdef CONFIG_PLIC_IRQ_AFFINITY
	/* Enable on secondary cores so that they can respond to PLIC */
	irq_enable(RISCV_IRQ_MEXT);
#endif /* CONFIG_PLIC_IRQ_AFFINITY */
#ifdef CONFIG_SOC_PER_CORE_INIT_HOOK
	soc_per_core_init_hook();
#endif /* CONFIG_SOC_PER_CORE_INIT_HOOK */
#ifdef CONFIG_SMP
	_kernel.cpus[cpu_num].arch.online = true;
#endif
	riscv_cpu_init[cpu_num].fn(riscv_cpu_init[cpu_num].arg);
}
