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
 * @file soc_isr_stacking.h
 *
 * @brief Syntacore SOC-specific stacking / unstacking operations during ISR
 *
 * @author amr-sc
 */

#ifndef SOC_ISR_STACKING_H
#define SOC_ISR_STACKING_H

/******************************************************************************
 *                              Defines section
 */
#ifdef _ASMLANGUAGE

#define SCR_DO_CALLER_SAVED(op) \
	RV_E(	op ra, __struct_arch_esf_ra_OFFSET(sp)	);\
\
	RV_E(	op t0, __struct_arch_esf_t0_OFFSET(sp)	);\
	RV_E(	op t1, __struct_arch_esf_t1_OFFSET(sp)	);\
	RV_E(	op t2, __struct_arch_esf_t2_OFFSET(sp)	);\
\
	RV_E(	op s0, __struct_arch_esf_s0_OFFSET(sp)	);\
	RV_E(	op s1, __struct_arch_esf_s1_OFFSET(sp)	);\
\
	RV_E(	op a0, __struct_arch_esf_a0_OFFSET(sp)	);\
	RV_E(	op a1, __struct_arch_esf_a1_OFFSET(sp)	);\
	RV_E(	op a2, __struct_arch_esf_a2_OFFSET(sp)	);\
	RV_E(	op a3, __struct_arch_esf_a3_OFFSET(sp)	);\
	RV_E(	op a4, __struct_arch_esf_a4_OFFSET(sp)	);\
	RV_E(	op a5, __struct_arch_esf_a5_OFFSET(sp)	);\
	RV_I(	op a6, __struct_arch_esf_a6_OFFSET(sp)	);\
	RV_I(	op a7, __struct_arch_esf_a7_OFFSET(sp)	);\
\
	RV_I(	op s2, __struct_arch_esf_s2_OFFSET(sp)	);\
	RV_I(	op s3, __struct_arch_esf_s3_OFFSET(sp)	);\
	RV_I(	op s4, __struct_arch_esf_s4_OFFSET(sp)	);\
	RV_I(	op s5, __struct_arch_esf_s5_OFFSET(sp)	);\
	RV_I(	op s6, __struct_arch_esf_s6_OFFSET(sp)	);\
	RV_I(	op s7, __struct_arch_esf_s7_OFFSET(sp)	);\
	RV_I(	op s8, __struct_arch_esf_s8_OFFSET(sp)	);\
	RV_I(	op s9, __struct_arch_esf_s9_OFFSET(sp)	);\
	RV_I(	op s10, __struct_arch_esf_s10_OFFSET(sp)	);\
	RV_I(	op s11, __struct_arch_esf_s11_OFFSET(sp)	);\
\
	RV_I(	op t3, __struct_arch_esf_t3_OFFSET(sp)	);\
	RV_I(	op t4, __struct_arch_esf_t4_OFFSET(sp)	);\
	RV_I(	op t5, __struct_arch_esf_t5_OFFSET(sp)	);\
	RV_I(	op t6, __struct_arch_esf_t6_OFFSET(sp)	)

#define SOC_ISR_SW_STACKING \
	/* Save caller-saved registers on current thread stack. */ \
	addi sp, sp, -__struct_arch_esf_SIZEOF; \
	SCR_DO_CALLER_SAVED(sr);

#define SOC_ISR_SW_UNSTACKING \
	/* Restore caller-saved registers from thread stack */ \
	SCR_DO_CALLER_SAVED(lr); \
	/* remove esf from the stack */ \
	addi sp, sp, __struct_arch_esf_SIZEOF;

#else

#define SOC_ISR_STACKING_ESF_DECLARE \
struct arch_esf { \
		unsigned long x0; \
\
		unsigned long ra;		/* return address */ \
		unsigned long sp;		/* preserved (user or kernel) stack pointer */ \
		unsigned long gp; \
		unsigned long tp; \
\
		unsigned long t0;		/* Caller-saved temporary register */ \
		unsigned long t1;		/* Caller-saved temporary register */ \
		unsigned long t2;		/* Caller-saved temporary register */ \
\
		unsigned long s0;		/* callee-saved s0 */ \
		unsigned long s1; \
\
		unsigned long a0;		/* function argument/return value */ \
		unsigned long a1;		/* function argument */ \
		unsigned long a2;		/* function argument */ \
		unsigned long a3;		/* function argument */ \
		unsigned long a4;		/* function argument */ \
		unsigned long a5;		/* function argument */ \
		unsigned long a6;		/* function argument */ \
		unsigned long a7;		/* function argument */ \
\
		unsigned long s2; \
		unsigned long s3; \
		unsigned long s4; \
		unsigned long s5; \
		unsigned long s6; \
		unsigned long s7; \
		unsigned long s8; \
		unsigned long s9; \
		unsigned long s10; \
		unsigned long s11; \
\
		unsigned long t3;		/* Caller-saved temporary register */ \
		unsigned long t4;		/* Caller-saved temporary register */ \
		unsigned long t5;		/* Caller-saved temporary register */ \
		unsigned long t6;		/* Caller-saved temporary register */ \
\
		unsigned long mepc;		/* machine exception program counter */ \
		unsigned long mstatus;	/* machine status register */ \
} __aligned(16)

#define SOC_ISR_STACKING_ESR_INIT

#endif /* _ASMLANGUAGE */

#endif /* SOC_ISR_STACKING_H */
