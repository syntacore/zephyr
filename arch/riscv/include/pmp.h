/*
 * Copyright (c) 2022 BayLibre, SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef PMP_H_
#define PMP_H_

void z_riscv_pmp_init(void);
void z_riscv_pmp_stackguard_prepare(struct k_thread *thread);
void z_riscv_pmp_stackguard_enable(struct k_thread *thread);
void z_riscv_pmp_usermode_init(struct k_thread *thread);
void z_riscv_pmp_usermode_prepare(struct k_thread *thread);
void z_riscv_pmp_usermode_enable(struct k_thread *thread);

/**
 * @brief Write a range of PMP entries to corresponding PMP registers
 *
 * PMP registers are accessed with the csr instruction which only takes an
 * immediate value as the actual register. This is performed more efficiently
 * in assembly code (pmp.S) than what is possible with C code.
 *
 * Requirement: start < end && end <= CONFIG_PMP_SLOTS
 *
 * @param start Start of the PMP range to be written
 * @param end End (exclusive) of the PMP range to be written
 * @param clear_trailing_entries True if trailing entries must be turned off
 * @param pmp_addr Array of pmpaddr values (starting at entry 0).
 * @param pmp_cfg Array of pmpcfg values (starting at entry 0).
 */
void z_riscv_write_pmp_entries(unsigned int start, unsigned int end,
				      bool clear_trailing_entries,
				      const unsigned long *pmp_addr,
				      const unsigned long *pmp_cfg);

/**
 * @brief Set PMP shadow register values in memory
 *
 * Register content is built using this function which selects the most
 * appropriate address matching mode automatically. Note that the special
 * case start=0 size=0 is valid and means the whole address range.
 *
 * @param index_p Location of the current PMP slot index to use. This index
 *                will be updated according to the number of slots used.
 * @param perm PMP permission flags
 * @param start Start address of the memory area to cover
 * @param size Size of the memory area to cover
 * @param pmp_addr Array of pmpaddr values (starting at entry 0).
 * @param pmp_cfg Array of pmpcfg values (starting at entry 0).
 * @param index_limit Index value representing the size of the provided arrays.
 * @return true on success, false when out of free PMP slots.
 */
bool z_riscv_set_pmp_entry(unsigned int *index_p, uint8_t perm,
			  uintptr_t start, size_t size,
			  unsigned long *pmp_addr, unsigned long *pmp_cfg,
			  unsigned int index_limit);

#endif /* PMP_H_ */
