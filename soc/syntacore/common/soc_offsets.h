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
 * @file soc_offsets.h
 *
 * @brief Syntacore SOC-specific stack frame offsets
 *
 * @author amr-sc
 */

#ifndef SOC_OFFSETS_H
#define SOC_OFFSETS_H

/******************************************************************************
 *                              Defines section
 */

/* Additional Syntacore-specific esf member offsets */
#define GEN_SOC_OFFSET_SYMS() \
	GEN_OFFSET_STRUCT(arch_esf, s1); \
	GEN_OFFSET_STRUCT(arch_esf, s2); \
	GEN_OFFSET_STRUCT(arch_esf, s3); \
	GEN_OFFSET_STRUCT(arch_esf, s4); \
	GEN_OFFSET_STRUCT(arch_esf, s5); \
	GEN_OFFSET_STRUCT(arch_esf, s6); \
	GEN_OFFSET_STRUCT(arch_esf, s7); \
	GEN_OFFSET_STRUCT(arch_esf, s8); \
	GEN_OFFSET_STRUCT(arch_esf, s9); \
	GEN_OFFSET_STRUCT(arch_esf, s10); \
	GEN_OFFSET_STRUCT(arch_esf, s11)

#endif /* SOC_OFFSETS_H */
