/*
 * @file soc_offsets.h
 *
 * @brief Syntacore SOC-specific stack frame offsets
 *
 * @author amr-sc
 *
 * Copyright (c) 2023 Syntacore. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SOC_OFFSETS_H
#define SOC_OFFSETS_H

/******************************************************************************
 *                              Defines section
 */

/* Additional Syntacore-specific esf member offsets */
#define GEN_SOC_OFFSET_SYMS() \
	GEN_OFFSET_SYM(z_arch_esf_t, s1); \
	GEN_OFFSET_SYM(z_arch_esf_t, s2); \
	GEN_OFFSET_SYM(z_arch_esf_t, s3); \
	GEN_OFFSET_SYM(z_arch_esf_t, s4); \
	GEN_OFFSET_SYM(z_arch_esf_t, s5); \
	GEN_OFFSET_SYM(z_arch_esf_t, s6); \
	GEN_OFFSET_SYM(z_arch_esf_t, s7); \
	GEN_OFFSET_SYM(z_arch_esf_t, s8); \
	GEN_OFFSET_SYM(z_arch_esf_t, s9); \
	GEN_OFFSET_SYM(z_arch_esf_t, s10); \
	GEN_OFFSET_SYM(z_arch_esf_t, s11)

#endif /* SOC_OFFSETS_H */
