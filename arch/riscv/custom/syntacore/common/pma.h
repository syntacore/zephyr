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
/*
 * @file pma.h
 *
 * @brief Syntacore Physical Memory Attributes interface
 *
 * @author amr-sc
 */

#ifndef SCR_PMA_H
#define SCR_PMA_H

/******************************************************************************
 *                              Includes section
 */
#include "soc.h"

/******************************************************************************
 *                              Defines section
 */
/* Max addressable memory range, starting from 0 */
#define ALL_MEMORY_BASE     (0x0UL)
#define ALL_MEMORY_SIZE     (0x0UL)         // covers all address space

/* PMA entry size in bits */
#define SCR_PMA_ENTRY_SIZE          (16)
/* PMA entry mask */
#define SCR_PMA_ENTRY_MASK          BIT_MASK(SCR_PMA_ENTRY_SIZE)
/* Number of PMA entry per pmacfgx CSR */
#define SCR_PMA_ENTRY_PER_CSR       ((__riscv_xlen) / (SCR_PMA_ENTRY_SIZE))

/* PMA region non-cacheable */
#define SCR_PMA_NC                  BIT(0)
/* PMA region strongly-ordered */
#define SCR_PMA_SO                  BIT(1)
/* PMA region non-idempotent */
#define SCR_PMA_NI                  BIT(2)
/* PMA region AMO */
#define SCR_PMA_AMO                 BIT(3)
/* PMA region reservability */
#define SCR_PMA_LR_SC               BIT(4)
/* PMA region lock bit */
#define SCR_PMA_L                   BIT(5)

/* PMA pmaxcfg legal values */

/* Cacheable idempotent main memory */
#define SCR_PMA_MEM_REGION_C_WO     (SCR_PMA_AMO | SCR_PMA_LR_SC)
/* Non-cacheable idempotent IO */
#define SCR_PMA_MEM_REGION_NC_SO    (SCR_PMA_NC | SCR_PMA_SO)
/* Non-cacheable idempotent main memory */
#define SCR_PMA_MEM_REGION_NC_WO    (SCR_PMA_NC)
/* Non-cacheable non-idempotent IO */
#define SCR_PMA_MEM_REGION_IO       (SCR_PMA_NC | SCR_PMA_SO | SCR_PMA_NI)

/* addressing mode bits */

/* PMA region address mode TOR */
#define SCR_PMA_A_TOR               BIT(6)
/* PMA region address mode NAPOT */
#define SCR_PMA_A_NAPOT             GENMASK(7,6)
/* PMA region address mode field null for disable region */
#define SCR_PMA_A_DISABLE           (~GENMASK(7,6))

/* PMA region start address shift */
#define SCR_PMA_ADDR_SHIFT          (2)
/* NAPOT addressing mode size shift */
#define SCR_PMA_NAPOT_SIZE_SHIFT    (3)

/* PMA CSRs */
#define SCR_CSR_PMA_BASE            (0x7E8)
#define SCR_CSR_PMACFG0             (SCR_CSR_PMA_BASE + 0)
#define SCR_CSR_PMACFG1             (SCR_CSR_PMA_BASE + 1)
#define SCR_CSR_PMACFG2             (SCR_CSR_PMA_BASE + 2)
#define SCR_CSR_PMACFG3             (SCR_CSR_PMA_BASE + 3)
#define SCR_CSR_PMACFG4             (SCR_CSR_PMA_BASE + 4)
#define SCR_CSR_PMACFG5             (SCR_CSR_PMA_BASE + 5)
#define SCR_CSR_PMACFG6             (SCR_CSR_PMA_BASE + 6)
#define SCR_CSR_PMACFG7             (SCR_CSR_PMA_BASE + 7)
#define SCR_CSR_PMAADDR0            (SCR_CSR_PMA_BASE + 8)
#define SCR_CSR_PMAADDR1            (SCR_CSR_PMA_BASE + 9)
#define SCR_CSR_PMAADDR2            (SCR_CSR_PMA_BASE + 10)
#define SCR_CSR_PMAADDR3            (SCR_CSR_PMA_BASE + 11)
#define SCR_CSR_PMAADDR4            (SCR_CSR_PMA_BASE + 12)
#define SCR_CSR_PMAADDR5            (SCR_CSR_PMA_BASE + 13)
#define SCR_CSR_PMAADDR6            (SCR_CSR_PMA_BASE + 14)
#define SCR_CSR_PMAADDR7            (SCR_CSR_PMA_BASE + 15)
#define SCR_CSR_PMAADDR8            (SCR_CSR_PMA_BASE + 16)
#define SCR_CSR_PMAADDR9            (SCR_CSR_PMA_BASE + 17)
#define SCR_CSR_PMAADDR10           (SCR_CSR_PMA_BASE + 18)
#define SCR_CSR_PMAADDR11           (SCR_CSR_PMA_BASE + 19)
#define SCR_CSR_PMAADDR12           (SCR_CSR_PMA_BASE + 20)
#define SCR_CSR_PMAADDR13           (SCR_CSR_PMA_BASE + 21)
#define SCR_CSR_PMAADDR14           (SCR_CSR_PMA_BASE + 22)
#define SCR_CSR_PMAADDR15           (SCR_CSR_PMA_BASE + 23)

/******************************************************************************
 *                              Prototypes section
 */
/**
 * @brief Performs PMA setup
 */
void scr_pma_setup(void);

/******************************************************************************
 *                              Implementation section
 */

#if IS_ENABLED(CONFIG_SCR_PMA)
/**
 * @brief Constructs pmaaddr register value
 *
 * @param addr - PMA region start address
 * @param size - PMA region size
 *
 * @return pmaaddr register value
 */
static inline uint64_t pma_construct_pmaaadr(uint64_t addr, uint64_t size)
{
	/*
	 * PMA address CSR contains address in [55:2] as well as size in NAPOT-mode
	 * with 4K as the minimum size value and 2^(xlen+3) as the maximum size value
	 */
	return (addr >> SCR_PMA_ADDR_SHIFT) | ((size >> SCR_PMA_NAPOT_SIZE_SHIFT) - 1);
}

/**
 * @brief Get number x of PMAxCFG CSR
 *
 * @param idx - PMA region index
 *
 * @return PMAxCFG CSR x number
 */
static inline size_t pma_get_pmaxcfg_num(int idx)
{
#if __riscv_xlen == 32
	return idx / SCR_PMA_ENTRY_PER_CSR;
#else
	return ((idx / SCR_PMA_ENTRY_PER_CSR) * 2);
#endif
}


/**
 * @brief Get offset in PMACFGx CSR
 *
 * @param idx - PMA region index
 *
 * @return offset in PMACFGx CSR
 */
static inline size_t pma_get_pmacfg_offset(int idx)
{
	return ((idx % SCR_PMA_ENTRY_PER_CSR) * SCR_PMA_ENTRY_SIZE);
}

/**
 * @brief Read PMACFGx CSR
 *
 * @param idx - PMA region index
 *
 * @return PMACFGx CSR value
 */
static inline size_t pma_read_pmacfg(int idx)
{
	size_t value = 0;

	switch (pma_get_pmaxcfg_num(idx)) {
		case 0: value = csr_read(SCR_CSR_PMACFG0); break;
		case 1: value = csr_read(SCR_CSR_PMACFG1); break;
		case 2: value = csr_read(SCR_CSR_PMACFG2); break;
		case 3: value = csr_read(SCR_CSR_PMACFG3); break;
		case 4: value = csr_read(SCR_CSR_PMACFG4); break;
		case 5: value = csr_read(SCR_CSR_PMACFG5); break;
		case 6: value = csr_read(SCR_CSR_PMACFG6); break;
		case 7: value = csr_read(SCR_CSR_PMACFG7); break;
		default: break;
	}

	return value;
}


/**
 * @brief Write PMACFGx CSR
 *
 * @param idx - PMA region index
 * @param value - PMACFGx CSR value
 */
static inline void pma_write_pmacfg(int idx, size_t value)
{
	switch (pma_get_pmaxcfg_num(idx)) {
		case 0: csr_write(SCR_CSR_PMACFG0, value); break;
		case 1: csr_write(SCR_CSR_PMACFG1, value); break;
		case 2: csr_write(SCR_CSR_PMACFG2, value); break;
		case 3: csr_write(SCR_CSR_PMACFG3, value); break;
		case 4: csr_write(SCR_CSR_PMACFG4, value); break;
		case 5: csr_write(SCR_CSR_PMACFG5, value); break;
		case 6: csr_write(SCR_CSR_PMACFG6, value); break;
		case 7: csr_write(SCR_CSR_PMACFG7, value); break;
		default: break;
	}
}

/**
 * @brief Read PMAADDRx CSR
 *
 * @param idx - PMA region index
 *
 * @return PMAADDRx CSR value
 */
static inline size_t pma_read_pmaaddr(int idx)
{
	size_t value = 0;

	switch (idx) {
		case 0: value = csr_read(SCR_CSR_PMAADDR0); break;
		case 1: value = csr_read(SCR_CSR_PMAADDR1); break;
		case 2: value = csr_read(SCR_CSR_PMAADDR2); break;
		case 3: value = csr_read(SCR_CSR_PMAADDR3); break;
		case 4: value = csr_read(SCR_CSR_PMAADDR4); break;
		case 5: value = csr_read(SCR_CSR_PMAADDR5); break;
		case 6: value = csr_read(SCR_CSR_PMAADDR6); break;
		case 7: value = csr_read(SCR_CSR_PMAADDR7); break;
		case 8: value = csr_read(SCR_CSR_PMAADDR8); break;
		case 9: value = csr_read(SCR_CSR_PMAADDR9); break;
		case 10: value = csr_read(SCR_CSR_PMAADDR10); break;
		case 11: value = csr_read(SCR_CSR_PMAADDR11); break;
		case 12: value = csr_read(SCR_CSR_PMAADDR12); break;
		case 13: value = csr_read(SCR_CSR_PMAADDR13); break;
		case 14: value = csr_read(SCR_CSR_PMAADDR14); break;
		case 15: value = csr_read(SCR_CSR_PMAADDR15); break;
		default: break;
	}

	return value;
}

/**
 * @brief Write PMAADDRx CSR
 *
 * @param idx - PMA region index
 * @param value - PMAADDRx CSR value
 */
static inline void pma_write_pmaaddr(int idx, size_t value)
{
	switch (idx) {
		case 0: csr_write(SCR_CSR_PMAADDR0, value); break;
		case 1: csr_write(SCR_CSR_PMAADDR1, value); break;
		case 2: csr_write(SCR_CSR_PMAADDR2, value); break;
		case 3: csr_write(SCR_CSR_PMAADDR3, value); break;
		case 4: csr_write(SCR_CSR_PMAADDR4, value); break;
		case 5: csr_write(SCR_CSR_PMAADDR5, value); break;
		case 6: csr_write(SCR_CSR_PMAADDR6, value); break;
		case 7: csr_write(SCR_CSR_PMAADDR7, value); break;
		case 8: csr_write(SCR_CSR_PMAADDR8, value); break;
		case 9: csr_write(SCR_CSR_PMAADDR9, value); break;
		case 10: csr_write(SCR_CSR_PMAADDR10, value); break;
		case 11: csr_write(SCR_CSR_PMAADDR11, value); break;
		case 12: csr_write(SCR_CSR_PMAADDR12, value); break;
		case 13: csr_write(SCR_CSR_PMAADDR13, value); break;
		case 14: csr_write(SCR_CSR_PMAADDR14, value); break;
		case 15: csr_write(SCR_CSR_PMAADDR15, value); break;
		default: break;
	}
}

/**
 * @brief Write PMA region attributes
 *
 * @param idx - PMA region index
 *
 * @param attr - PMA region attributes
 */
static inline void pma_write_attr(int idx, uint16_t attr)
{
	/* Read the configuration register according to PMA entry */
	 size_t pmacfg = pma_read_pmacfg(idx);
	 const size_t offset = pma_get_pmacfg_offset(idx);

	/* Clean needed entry attributes but not entire CSR */
	 pmacfg &= ~(SCR_PMA_ENTRY_MASK << offset);

	/* Write needed entry attributes but not entire CSR */
	 pmacfg |= ((attr & SCR_PMA_ENTRY_MASK) << offset);

	/* Write new value to configuration register according to PMA entry */
	 pma_write_pmacfg(idx, pmacfg);
}


/**
 * @brief Read PMA region attributes
 *
 * @param idx - PMA region index
 *
 * @return PMA region attributes
 */
static inline uint16_t pma_read_attr(int idx)
{
	size_t pmacfg = pma_read_pmacfg(idx);

	return ((pmacfg >> pma_get_pmacfg_offset(idx)) & SCR_PMA_ENTRY_MASK);
}


/**
 * @brief Disable PMA region
 *
 * @param idx - PMA region index
 */
static inline void pma_disable_region(int idx)
{
	pma_write_attr(idx, pma_read_attr(idx) & SCR_PMA_A_DISABLE);
}


/**
 * @brief Configure PMA region
 *        Supports only NAPOT addressing mode
 *
 * @param idx - PMA region index
 * @param addr - PMA region start address
 * @param size - PMA region size
 * @param attr - PMA region attributes
 */
static inline void pma_configure_region(int idx, uint64_t addr,
	uint64_t size, uint16_t attr)
{
	const uint64_t pmaaddr = pma_construct_pmaaadr(addr, size);

	/* PMA address CSR must be set with the disabled configuration register */
	pma_disable_region(idx);

	/* Write PMA address CSR */
	pma_write_pmaaddr(idx, pmaaddr);

	/* Write PMA configuration register */
	pma_write_attr(idx, attr | SCR_PMA_A_NAPOT);
}

/**
 * @brief Performs PMA early initialization.
 * 		Brings platform to safe default state
 */
static inline void scr_pma_early_init(void)
{
	/* Protect default (last) entry and configure access to MMCFG */
	pma_configure_region(CONFIG_SCR_PMA_ENTRIES_COUNT - 2, PLF_MMCFG_BASE,
		PLF_MMCFG_SIZE, SCR_PMA_MEM_REGION_IO);
}
#endif /* CONFIG_SCR_PMA */

#endif /* SCR_PMA_H */
