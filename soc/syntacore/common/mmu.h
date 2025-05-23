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
 * @file mmu.h
 *
 * @brief Syntacore MMU interface
 *
 * @author amr-sc
 */

#ifndef SCR_MMU_H
#define SCR_MMU_H

/******************************************************************************
 *                              Defines section
 */

#define CSR_MMU_BASE   0xbc0
#define CSR_MMU_PATTR  (CSR_MMU_BASE + 0)
#define CSR_MMU_VADDR  (CSR_MMU_BASE + 1)
#define CSR_MMU_UPDATE (CSR_MMU_BASE + 2)
#define CSR_MMU_SCAN   (CSR_MMU_BASE + 3)

#define CSR_MMU_UPDATE_TLBI  (1UL << 0)
#define CSR_MMU_UPDATE_TLBD  (1UL << 1)
#define CSR_MMU_UPDATE_MPAGE (1UL << 2)
#define CSR_MMU_UPDATE_GPAGE (2UL << 2)
#define CSR_MMU_UPDATE_TPAGE (3UL << 2)

#define CSR_MMU_PATTR_V (1UL << 0)
#define CSR_MMU_PATTR_R (1UL << 1)
#define CSR_MMU_PATTR_W (1UL << 2)
#define CSR_MMU_PATTR_X (1UL << 3)
#define CSR_MMU_PATTR_M (1UL << 8)

#define CSR_MMU_SCAN_SEL_TLBI (0UL << 31)
#define CSR_MMU_SCAN_SEL_TLBD (1UL << 31)

#define SATP_MODE       0x8000000000000000UL /* SV39 */
#define SATP_PPN        0x00000FFFFFFFFFFFUL
#define SATP_ASID_MASK  0xFFFFUL
#define SATP_ASID_SHIFT 44

#define PAGE_VALID    (1 << 0) /* Valid entry */
#define PAGE_READ     (1 << 1) /* Readable */
#define PAGE_WRITE    (1 << 2) /* Writable */
#define PAGE_EXEC     (1 << 3) /* Executable */
#define PAGE_USER     (1 << 4) /* User */
#define PAGE_GLOBAL   (1 << 5) /* Global */
#define PAGE_ACCESSED (1 << 6) /* Set by hardware on any access */
#define PAGE_DIRTY    (1 << 7) /* Set by hardware on any write */

#define PAGE_COMMON     (PAGE_ACCESSED | PAGE_DIRTY)
#define PAGE_RWX        (PAGE_VALID | PAGE_READ | PAGE_WRITE | PAGE_EXEC)
#define PAGE_RW         (PAGE_VALID | PAGE_READ | PAGE_WRITE)
#define PAGE_NEXT_TABLE (PAGE_VALID)

#define PAGE_SHIFT (12)
#define PAGE_SIZE  ((1UL) << PAGE_SHIFT)

#define PFN(x) ((x) >> PAGE_SHIFT)

#define PPN2_SHIFT     30
#define PPN2_MASK      0x3ffffff
#define PPN2_PTE_SHIFT 28

#define VPN2_SHIFT 30
#define VPN2_MASK  0x1ff

#define PPN1_SHIFT     21
#define PPN1_MASK      0x1ff
#define PPN1_PTE_SHIFT 19

#define VPN1_SHIFT 21
#define VPN1_MASK  0x1ff

#define PPN0_SHIFT     12
#define PPN0_MASK      0x1ff
#define PPN0_PTE_SHIFT 10

#define VPN0_SHIFT 12
#define VPN0_MASK  0x1ff

#define VPN2(x) (((x) >> VPN2_SHIFT) & VPN2_MASK)
#define VPN1(x) (((x) >> VPN1_SHIFT) & VPN1_MASK)
#define VPN0(x) (((x) >> VPN0_SHIFT) & VPN0_MASK)

#define PGD(x) (PFN(x) << PPN0_PTE_SHIFT)
#define PMD(x) (PFN(x) << PPN0_PTE_SHIFT)
#define PTE(x)                                                                                                         \
    (((((x) >> PPN2_SHIFT) & PPN2_MASK) << PPN2_PTE_SHIFT) | ((((x) >> PPN1_SHIFT) & PPN1_MASK) << PPN1_PTE_SHIFT) |   \
     ((((x) >> PPN0_SHIFT) & PPN0_MASK) << PPN0_PTE_SHIFT))

typedef struct
{
    unsigned long pgd;
} pgd_t;

#define PTRS_PER_PGD (PAGE_SIZE / sizeof(pgd_t))

static inline int pgd_valid(pgd_t const* pgd) { return ((pgd->pgd & PAGE_RWX) == PAGE_VALID); }

typedef struct
{
    unsigned long pmd;
} pmd_t;

#define PTRS_PER_PMD (PAGE_SIZE / sizeof(pmd_t))
#define MEGA_PAGE_SIZE (PAGE_SIZE * PTRS_PER_PMD) /* 4M */
#define GIGA_PAGE_SIZE (MEGA_PAGE_SIZE * PTRS_PER_PGD) /* 1G */

#define PMD_COUNT(size) ((size) / MEGA_PAGE_SIZE)
#define PTE_COUNT(size) ((size) / PAGE_SIZE)
/* Limit address translation to 256M */
#define MMU_TRANSLATED_DDR_SIZE (256 * 1024 * 1024)

static inline int pmd_valid(pmd_t const* pmd) { return ((pmd->pmd & PAGE_RWX) == PAGE_VALID); }

static inline pmd_t* pgd_to_pmd(pgd_t const* pgd, unsigned long idx)
{
    return (pmd_t*)(((pgd->pgd >> PPN0_PTE_SHIFT) << PAGE_SHIFT) + idx * sizeof(*pgd));
}

typedef struct
{
    unsigned long pte;
} pte_t;

#define PTRS_PER_PTE (PAGE_SIZE / sizeof(pte_t))

static inline int pte_valid(pte_t const* pte) { return ((pte->pte & PAGE_VALID) == PAGE_VALID); }

static inline pte_t* pmd_to_pte(pmd_t const* pmd, unsigned long idx)
{
    return (pte_t*)(((pmd->pmd >> PPN0_PTE_SHIFT) << PAGE_SHIFT) + idx * sizeof(*pmd));
}

static inline unsigned long* pte_to_paddr(pte_t const* pte)
{
    return (unsigned long*)((pte->pte >> PPN0_PTE_SHIFT) << PAGE_SHIFT);
}

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Runs function in S-Mode
 */
void mmu_run_in_s_mode(void (func)(void));

#endif /* SCR_MMU_H */
