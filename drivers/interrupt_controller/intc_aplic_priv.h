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

#ifndef ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_APLIC_PRIV_H_
#define ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_APLIC_PRIV_H_

#include <zephyr/kernel.h>

#define APLIC_BASE_ADDR           DT_INST_REG_ADDR_U64(0)
#define APLIC_MAX_PRIO            DT_INST_PROP(0, riscv_max_priority)

/**
 * @brief Set APLIC register value
 *
 * @param[in] reg - APLIC register
 * @param[in] value - value to set
 */
static inline void aplic_set(size_t reg, uint32_t value)
{
	sys_write32(value, APLIC_BASE_ADDR + reg);
}

/**
 * @brief Get APLIC register value
 *
 * @param[in] reg - APLIC register
 *
 * @result APLIC register value
*/
static inline uint32_t aplic_get(size_t reg)
{
	return sys_read32(APLIC_BASE_ADDR + reg);
}

#define APLIC_DOMAINCFG           0x0000
#define APLIC_DOMAINCFG_IE        BIT(8)
#define APLIC_DOMAINCFG_DM        BIT(2)
#define APLIC_DOMAINCFG_DM_MSI    1

/**
 * @brief Set domain config
 *
 * @param[in] domaincfg new value of domaincfg
 *
 * An APLIC supports one or more interrupt domains, each associated with a subset
 * of RISC-V harts at one privilege level (machine or supervisor level).
 * The harts within an interrupt domain are those that the domain can interrupt
 * at the corresponding privilege level. Each domain has its own memory-mapped
 * control region in the machine’s address space that appears to control a complete,
 * separate APLIC, though in fact all domain interfaces together access a single
 * combined interrupt controller.
 */
static inline void aplic_set_domaincfg(uint32_t domaincfg)
{
	aplic_set(APLIC_DOMAINCFG, domaincfg);
}

/**
 * @brief APLIC Get domain config
 *
 * An APLIC supports one or more interrupt domains, each associated with a subset of
 * RISC-V harts at one privilege level (machine or supervisor level). The harts within
 * an interrupt domain are those that the domain can interrupt at the corresponding privilege
 * level. Each domain has its own memory-mapped control region in the machine’s address space
 * that appears to control a complete, separate APLIC, though in fact all domain interfaces
 * together access a single combined interrupt controller.
 *
 * @result domaincfg from @ref APLIC_DOMAINCFG
 */
static inline uint32_t aplic_get_domaincfg(void)
{
	return aplic_get(APLIC_DOMAINCFG);
}

/**
 * @brief minimum value for an interrupt source
 *
 * @details Each of interrupt sources of an APLIC has a fixed unique identity number in the
 * range of 1 to N, where N is the total number of sources at the APLIC.
 * Number 0 (zero) is not a valid interrupt identity number at an APLIC.
 * The maximum number of interrupt sources an APLIC can support is 1023.
 */
#define APLIC_SOURCE_MIN   (1)

/**
 * @brief maximum number for interrupt sources
 */
#define APLIC_SOURCE_MAX   (DT_INST_PROP(0, riscv_num_sources))

/**
 * @brief APLIC sourcecfg_base[1..1023]
 */
#define APLIC_SOURCECFG_BASE      (0x0004)

#define APLIC_SOURCECFG_D         BIT(10)
#define APLIC_SOURCECFG_CHILD_IDX GENMASK(9, 0)
#define APLIC_SOURCECFG_SM        GENMASK(2, 0)

/**
 * @brief Source Mode filed values - Inactive
 *
 * @details Inactive in this domain (and not delegated)
 */
#define APLIC_SOURCECFG_SM_INACTIVE (0)

/**
 * @brief Source Mode filed values - Detach
 *
 * @details Active, detached from the source wire
 */
#define APLIC_SOURCECFG_SM_DETACH   (1)

/**
 * @brief Source Mode filed values - rising edge
 *
 * @details Active, edge-sensitive; asserted on rising edge
*/
#define APLIC_SOURCECFG_SM_EDGE1    (4)

/**
 * @brief Set Source Config
 *
 * @param[in] src source to get (1..1023)
 * @param[in] sourcecfg new value to set
 */

static inline void aplic_set_sourcecfg(size_t src, uint32_t sourcecfg)
{
    aplic_set(APLIC_SOURCECFG_BASE + (src - 1) * sizeof(uint32_t), sourcecfg);
}

/**
 *
 * @brief Machine MSI address configuration
 *
 * @details For machine-level interrupt domains, register mmsiaddrcfg can optionally
 * provide parameters used to determine the addresses to write outgoing MSIs.
 * If no interrupt domain of the APLIC supports the MSI delivery mode
 * (domaincfg.DM is a read-only zero for all domains), this register is not
 * implemented for any domain. Otherwise, it is implemented for the root domain,
 * and may or may not be implemented for other machine-level domains.
 * For the domains not at the machine level, it is never implemented. When a domain does not
 * implement mmsiaddrcfg and mmsiaddrcfgh, the eight bytes at their locations are
 * simply read-only zeros like the other reserved bytes.
 */

/**
 * @brief mmsiaddrcfg[31:0] register offset in the memory-mapped control region for an interrupt
 * domain
 *
 * @details mmsiaddrcfg[31:0] Low Base PPN (Physical Page Number)
 */
#define APLIC_MMSIADDRCFG  (0x1BC0)
/**
 * @brief mmsiaddrcfgh register offset in the memory-mapped control region for an interrupt
 * domain
 *
 * @details Fields High Base PPN from mmsiaddrcfgh and Low Base PPN from mmsiaddrcfg
 * concatenate to form a 44-bit Base PPN (Physical Page Number).
 */
#define APLIC_MMSIADDRCFGH (0x1BC4)

/**
 * @brief mmsiaddrcfgh[31] L (Locked) shift
 *
 * @details affects smsiaddrcfg(h) too
 */
#define APLIC_MMSIADDRCFGH_LOCKED_SHIFT (31)
/**
 * @brief mmsiaddrcfgh[31] L (Locked) mask
 */
#define APLIC_MMSIADDRCFGH_LOCKED_MASK  (0x1)
/**
 * @brief mmsiaddrcfgh[31] L (Locked) bit
 */
#define APLIC_MMSIADDRCFGH_L_BIT        BIT(APLIC_MMSIADDRCFGH_LOCKED_SHIFT)
/**
 * @brief mmsiaddrcfgh[28:24] HHXS (High Hart Index Shift) shift
 */
#define APLIC_MMSIADDRCFGH_HHXS_SHIFT   (24)
/**
 * @brief mmsiaddrcfgh[28:24] HHXS (High Hart Index Shift) mask
 */
#define APLIC_MMSIADDRCFGH_HHXS_MASK    (0x1f)
/**
 * @brief mmsiaddrcfgh[22:20] LHXS (Low Hart Index Shift) shift
 */
#define APLIC_MMSIADDRCFGH_LHXS_SHIFT   (20)
/**
 * @brief mmsiaddrcfgh[22:20] LHXS (Low Hart Index Shift) mask
 */
#define APLIC_MMSIADDRCFGH_LHXS_MASK    (0x7)
/**
 * @brief mmsiaddrcfgh[18:16] HHXW (High Hart Index Width) shift
 */
#define APLIC_MMSIADDRCFGH_HHXW_SHIFT   (16)
/**
 * @brief mmsiaddrcfgh[18:16] HHXW (High Hart Index Width) mask
 */
#define APLIC_MMSIADDRCFGH_HHXW_MASK    (0x7)
/**
 * @brief msiaddrcfgh[15:12] LHXW (Low Hart Index Width) shift
 */
#define APLIC_MMSIADDRCFGH_LHXW_SHIFT   (12)
/**
 * @brief msiaddrcfgh[15:12] LHXW (Low Hart Index Width) mask
 */
#define APLIC_MMSIADDRCFGH_LHXW_MASK    (0xF)
/**
 * @brief mmsiaddrcfgh[11:0] High Base PPN (Physical Page Number) shift
 */
#define APLIC_MMSIADDRCFGH_HBPPN_SHIFT  (0)
/**
 * @brief mmsiaddrcfgh[11:0] High Base PPN (Physical Page Number) mask
 */
#define APLIC_MMSIADDRCFGH_HBPPN_MASK   (0xfff)

#define IMSIC_MMIO_PAGE_SHIFT           (12)
#define APLIC_LHXW_M                    (3)
#define APLIC_LHXS_M                    (0)
#define APLIC_HHXW_M                    (0)
#define APLIC_HHXS_M                    (2 * IMSIC_MMIO_PAGE_SHIFT)
#define APLIC_xMSIADDRCFG_PPN_SHIFT     (12)

#define APLIC_xMSIADDRCFG_PPN_HART(__lhxs) \
        ((1UL << (__lhxs)) - 1)
#define APLIC_xMSIADDRCFG_PPN_LHX_MASK(__lhxw) \
        ((1UL << (__lhxw)) - 1)
#define APLIC_xMSIADDRCFG_PPN_LHX_SHIFT(__lhxs) \
        ((__lhxs))
#define APLIC_xMSIADDRCFG_PPN_LHX(__lhxw, __lhxs) \
        (APLIC_xMSIADDRCFG_PPN_LHX_MASK(__lhxw) << \
         APLIC_xMSIADDRCFG_PPN_LHX_SHIFT(__lhxs))

#define APLIC_xMSIADDRCFG_PPN_HHX_MASK(__hhxw) \
        ((1UL << (__hhxw)) - 1)
#define APLIC_xMSIADDRCFG_PPN_HHX_SHIFT(__hhxs) \
        ((__hhxs) + APLIC_xMSIADDRCFG_PPN_SHIFT)
#define APLIC_xMSIADDRCFG_PPN_HHX(__hhxw, __hhxs) \
        (APLIC_xMSIADDRCFG_PPN_HHX_MASK(__hhxw) << \
         APLIC_xMSIADDRCFG_PPN_HHX_SHIFT(__hhxs))
/**
 * @brief MSI configuration data
 */
struct aplic_msicfg_data {
	unsigned long lhxs; //!< Low Hart Index Shift
	unsigned long lhxw; //!< Low Hart Index Width
	unsigned long hhxs; //!< High Hart Index Shift
	unsigned long hhxw; //!< High Hart Index Width
	unsigned long base_addr; //!< Base Address
};

/**
 * @brief Write APLIC MSI config
 *
 * @param[in] msicfg data to write
 */
static inline void aplic_write_msiconfig(struct aplic_msicfg_data *msicfg)
{
	uint32_t val;
	unsigned long base_ppn;
	size_t msicfgaddr = APLIC_BASE_ADDR + APLIC_MMSIADDRCFG;
	size_t msicfgaddrh = APLIC_BASE_ADDR + APLIC_MMSIADDRCFGH;

	/* Check if MSI config is already locked */
	if (sys_read32(msicfgaddrh) &
			(APLIC_MMSIADDRCFGH_LOCKED_MASK << APLIC_MMSIADDRCFGH_LOCKED_SHIFT))
		return;

	base_ppn = msicfg->base_addr >> APLIC_xMSIADDRCFG_PPN_SHIFT;
	base_ppn &= ~APLIC_xMSIADDRCFG_PPN_HART(msicfg->lhxs);
	base_ppn &= ~APLIC_xMSIADDRCFG_PPN_LHX(msicfg->lhxw, msicfg->lhxs);
	base_ppn &= ~APLIC_xMSIADDRCFG_PPN_HHX(msicfg->hhxw, msicfg->hhxs);

	sys_write32((uint32_t)base_ppn, msicfgaddr);

	val = (((uint64_t)base_ppn) >> 32) &
		APLIC_MMSIADDRCFGH_HBPPN_MASK;
	val |= (msicfg->lhxw & APLIC_MMSIADDRCFGH_LHXW_MASK)
		<< APLIC_MMSIADDRCFGH_LHXW_SHIFT;
	val |= (msicfg->hhxw & APLIC_MMSIADDRCFGH_HHXW_MASK)
		<< APLIC_MMSIADDRCFGH_HHXW_SHIFT;
	val |= (msicfg->lhxs & APLIC_MMSIADDRCFGH_LHXS_MASK)
		<< APLIC_MMSIADDRCFGH_LHXS_SHIFT;
	val |= (msicfg->hhxs & APLIC_MMSIADDRCFGH_HHXS_MASK)
		<< APLIC_MMSIADDRCFGH_HHXS_SHIFT;

	sys_write32(val, msicfgaddrh);
}

/**
 * @brief Minimum register bit for interrupts
 *
 * @details relates to setip[], in_clrip[], setie[], clrie[] registers
 */
#define APLIC_MIN_IRQ_REG (0)

/**
 * @brief Maximum register bit for interrupts
 *
 * @details relates to setip[], in_clrip[], setie[], clrie[] registers
 */
#define APLIC_MAX_IRQ_REG (31)

/**
 * @brief Set interrupt-enable bits (setie[0]–setie[31])
 */
#define APLIC_SETIE_BASE (0x1e00)

/**
 * @brief get setie
 *
 * @param[in] reg_num register within  APLIC_MIN_IRQ_REG .. APLIC_MAX_IRQ_REG
 *
 * @return value if setie[reg_num] in the memory-mapped control region for the domain
 */
static inline uint32_t aplic_get_setie(size_t reg_num)
{
	return aplic_get(APLIC_SETIE_BASE + reg_num * sizeof(uint32_t));
}

/**
 * @brief Check if IRQ is enabled for particular source
 *
 * @param[in] src - interrupt source number
 *
 * @return 1 if IRQ is enabled for particular source; 0 - otherwise
 */
static inline uint32_t aplic_is_irq_enabled(size_t src)
{
	size_t reg_num = ((src - 1) / 32);
	uint32_t setie = aplic_get_setie(reg_num);

	return !!(setie & (1 << (src & 31)));
}

/**
 * @brief Set interrupt-enable bit by number (setienum)
 */
#define APLIC_SETIENUM   (0x1edc)

/**
 * @brief set setienum
 *
 * @param[in] src - interrupt source number
*/
static inline void aplic_set_setienum(size_t src)
{
	aplic_set(APLIC_SETIENUM, src);
}

/**
 * @brief Clear interrupt-enable bits (clrie[0]–clrie[31])
 */
#define APLIC_CLRIE_BASE (0x1f00)

#define APLIC_CLRIE_CLR_ALL	(0xffffffffUL)

/**
 * @brief Set clrie
 *
 * @param[in] reg_num register within  APLIC_MIN_IRQ_REG .. APLIC_MAX_IRQ_REG
 * @param[in] value if clrie[reg_num] in the memory-mapped control region for the domain
 */
static inline void aplic_set_clrie(size_t reg_num, uint32_t value)
{
	aplic_set(APLIC_CLRIE_BASE + reg_num * sizeof(uint32_t), value);
}

/**
 * @brief Clear interrupt-enable bit by number (clrienum)
 */
#define APLIC_CLRIENUM   (0x1fdc)

/**
 * @brief set clrienum
 *
 * @param[in] src - interrupt source number
 */
static inline void aplic_set_clrienum(size_t src)
{
	aplic_set(APLIC_CLRIENUM, src);
}

/**
 * @brief Interrupt delivery control (IDC)
 *
 * @details an interrupt domain is in the direct delivery mode (domaincfg.DM = 0),
 * interrupts are delivered from the APLIC to harts by a unique signal to each hart,
 * usually via a dedicated wire. In this case, the domain’s memory-mapped control
 * region contains an array of interrupt delivery control (IDC) structures at the end,
 * one IDC structure per potential hart index. The first IDC structure is for the
 * domain’s hart with index 0; the second is for the hart with index 1; etc.
 *
 */
#define APLIC_IDC_SIZE	(0x20)

/**
 * @brief Interrupt delivery control (IDC) base offset
 */
#define APLIC_IDC_BASE (0x4000)

/**
 * @brief Get offset of Interrupt delivery control (IDC) entity within APLIC
 * 	memory space
 *
 * @param[in] hart hart ID
 *
 * @return IDC entity offset
 */
static inline uintptr_t aplic_get_idc_entity_offset(size_t hart)
{
	return (uintptr_t)(APLIC_IDC_BASE + APLIC_IDC_SIZE * hart);
}

/**
 * @brief Interrupt delivery enable (idelivery)
 */
#define APLIC_IDC_IDELIVERY         (0x00)

#define APLIC_IDC_IDELIVERY_DISABLE 0x00
#define APLIC_IDC_IDELIVERY_ENABLE  0x01

/**
 * @brief Set idelivery field from Interrupt delivery control (IDC) structure
 *
 * @param[in] hart hart ID
 * @param[in] idelivery idelivery value to set
 */
static inline void aplic_set_idc_idelivery(size_t hart, uint32_t idelivery)
{
	uintptr_t idc_offset = aplic_get_idc_entity_offset(hart);

	aplic_set(idc_offset + APLIC_IDC_IDELIVERY, idelivery);
}

/**
 * @brief Interrupt enable threshold (ithreshold)
 * @ingroup aia_idc
*/
#define APLIC_IDC_ITHRESHOLD        (0x08)

#define APLIC_IDC_ITHRESHOLD_DISABLE 0x00

/**
 * @brief Set ithreshold field from Interrupt delivery control (IDC) structure
 *
 * @param[in] hart hart ID
 * @param[in] ithreshold ithreshold value to set
 */
static inline void aplic_set_idc_ithreshold(size_t hart, uint32_t ithreshold)
{
	uintptr_t idc = aplic_get_idc_entity_offset(hart);

	aplic_set(idc + APLIC_IDC_ITHRESHOLD, ithreshold);
}

#define APLIC_IDC_IRQ_ID          GENMASK(25, 16)
#define APLIC_IDC_IRQ_PRIO        GENMASK(7, 0)

/**
 * @brief Claim top interrupt (claimi)
 *
 * @details Register claimi has the same value as topi. When this value
 * is not zero, reading claimi has the simultaneous side effect of clearing
 * the pending bit for the reported interrupt identity if possible.
*/
#define APLIC_IDC_CLAIMI (0x1c)

/**
 * @brief Get claimi field from Interrupt delivery control (IDC) structure
 *
 * @param[in] hart hart ID
 *
 * @return claimi
 */
static inline uint32_t aplic_get_idc_claimi(size_t hart)
{
	uintptr_t idc = aplic_get_idc_entity_offset(hart);

	return aplic_get(idc + APLIC_IDC_CLAIMI);
}

/**
 * @brief Interrupt targets (target[1]-target[1023]
 */
/**
 * @brief  target_base[1..1023] in the memory-mapped control region for the domain
 */
#define APLIC_TARGET_BASE         (0x3004)

#define APLIC_TARGET_HART_IDX     GENMASK(31, 18)
#define APLIC_TARGET_IPRIO        GENMASK(7, 0)
#define APLIC_TARGET_EIID         GENMASK(10, 0)

/**
 * @brief make MSI target
 *
 * @param[in] hart_idx target[31:18] Hart index
 * @param[in] eiid target[10:0]  Ext Interrupt Identity
 *
 * @return target value
 */
static inline uint32_t aplic_make_msi_target(uint32_t hart_idx, uint32_t eiid)
{
	return FIELD_PREP(APLIC_TARGET_HART_IDX, hart_idx) | FIELD_PREP(APLIC_TARGET_EIID, eiid);
}

/**
 * @brief make MSI target
 *
 * @param[in] hart_idx target[31:18] Hart index
 * @param[in] iprio target[7:0]  Interrupt priority
 *
 * @return target value
 */
static inline uint32_t aplic_make_direct_target(uint32_t hart_idx, uint32_t iprio)
{
	return FIELD_PREP(APLIC_TARGET_HART_IDX, hart_idx) | FIELD_PREP(APLIC_TARGET_IPRIO, iprio);
}

/**
 * @brief set target
 *
 * @param[in] src - interrupt source number
 * @param[in] target new value for target[src] for domain
 */
static inline void aplic_set_target(size_t src, uint32_t target)
{
	aplic_set(APLIC_TARGET_BASE + (src - 1) * sizeof(uint32_t), target);
}

/**
 * @brief get target
 *
 * @param[in] src - interrupt source number
 *
 * @return target[scr] for domain
 */
static inline uint32_t aplic_get_target(size_t src)
{
	return aplic_get(APLIC_TARGET_BASE + (src - 1) * sizeof(uint32_t));
}

#endif /* ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_APLIC_PRIV_H_ */
