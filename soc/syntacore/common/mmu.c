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
 * @file mmu.c
 *
 * @brief Syntacore MMU implementation
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/riscv/csr.h>
#include <zephyr/devicetree.h>
#include "mmu.h"

/******************************************************************************
 *                              Defines section
 */
#define DDR_BASE_ADDR (DT_REG_ADDR_U64(DT_NODELABEL(ddr)))
BUILD_ASSERT((DDR_BASE_ADDR % MEGA_PAGE_SIZE) == 0, "Invalid DDR base address alignment");

#if IS_ENABLED(CONFIG_SCR_MMU)

/******************************************************************************
 *                              Prototypes section
 */
extern void z_riscv_run_in_s_mode(void (func)(void));

/******************************************************************************
 *                              Static variables section
 */
static struct k_spinlock pt_lock;
static pgd_t root_pgd[PTRS_PER_PGD] __attribute__((aligned(PAGE_SIZE)));

static pmd_t ddr_pmd[ROUND_UP(PMD_COUNT(MMU_TRANSLATED_DDR_SIZE), PTRS_PER_PMD)] __attribute__((aligned(PAGE_SIZE)));
extern pte_t ddr_pte[ROUND_UP(PTE_COUNT(MMU_TRANSLATED_DDR_SIZE), PTRS_PER_PTE)] __attribute__((aligned(PAGE_SIZE)));

/******************************************************************************
 *                              Implementation section
 */
/**
 * @brief Sets up direct VA->PA mapping page tables
 */
static void mmu_setup_page_table(void)
{
	k_spinlock_key_t key = k_spin_lock(&pt_lock);

	/* Page table is already prepared - leave */
	if (root_pgd[VPN2(DDR_BASE_ADDR)].pgd != 0)
		goto exit;

	/* 256M DDR: 65536 * 4K pages or 128 * 2M pages */
	root_pgd[VPN2(DDR_BASE_ADDR)].pgd = PGD((unsigned long)&ddr_pmd[0]) | PAGE_NEXT_TABLE;
	for (int i = 0; i < PMD_COUNT(MMU_TRANSLATED_DDR_SIZE); i++) {
		ddr_pmd[VPN1(DDR_BASE_ADDR + i * MEGA_PAGE_SIZE)].pmd =
		PMD((unsigned long)&ddr_pte[0] + i * PAGE_SIZE) | PAGE_NEXT_TABLE;
	}

	/* Provide one-to-one VA->PA translation in case no external page table is provided */
	if (ddr_pte[0].pte == 0) {
		for (int i = 0; i < PTE_COUNT(MMU_TRANSLATED_DDR_SIZE); i++) {
			ddr_pte[(i / PTRS_PER_PTE) * PTRS_PER_PTE + VPN0(DDR_BASE_ADDR + i * PAGE_SIZE)].pte =
				PTE(DDR_BASE_ADDR + i * PAGE_SIZE) | PAGE_COMMON | PAGE_RWX;
		}
	}

exit:
	k_spin_unlock(&pt_lock, key);
}

/**
 * @brief Enables paging. Paging is applicable to S/U modes only
 */
static inline void mmu_enable_paging(void)
{
	__asm__ volatile("sfence.vma" ::: "memory");
	csr_write(satp, PFN((unsigned long)&root_pgd[0]) | SATP_MODE);
}

/**
 * @brief Disables paging
 */
static inline void mmu_disable_paging(void)
{
	csr_write(satp, 0);
}

/**
 * @brief S-Mode configuration
 */
static void mmu_setup_s_mode(void)
{
	/* disable paging */
	mmu_disable_paging();

	/* disable interrupts delegations */
	csr_write(mideleg, 0);

	/* setup exceptions delegations */
	csr_write(medeleg, 0);

	/* disable interrupts in S mode */
	csr_write(stvec, 0);
	csr_write(sie, 0);
	csr_write(sip, 0);

	/* set MPP=0, SIE=0, SPP=U */
	const unsigned long ms =
		csr_read(mstatus) & ~(MSTATUS_MPP | MSTATUS_SPP | MSTATUS_SIE);
	/* MPP=S */
	csr_write(mstatus, ms | (MSTATUS_MPP_S));
}

/**
 * @brief Runs function in S-Mode
 */
void mmu_run_in_s_mode(void (func)(void))
{
	mmu_setup_s_mode();
	mmu_setup_page_table();
	mmu_enable_paging();

	/* Switch to S-mode and run the function there */
	z_riscv_run_in_s_mode(func);
	/* Here we are back to M-mode again */
}

#endif /* CONFIG_SCR_MMU */
