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
 * @file iccm_v2.c
 *
 * @brief Syntacore Inter-core communication module ver.2 (ICCMv2) support
 *
 * @author amr-sc
 */

/******************************************************************************
 *                              Includes section
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/arch/cpu.h>
#include <soc.h>
#include <ipi.h>

/******************************************************************************
 *                              Defines section
 */
#define SCR_ICCM_MSG_SCHEDULE 1

/* SEGMENT A */
#define SCR_ICCM_SEG_A_BASE                         (PLF_ICCM_BASE)
#define SCR_ICCM_SEG_A_BUFSTATUS                    (SCR_ICCM_SEG_A_BASE + 0)
#define SCR_ICCM_SEG_A_BUFREAD                      (SCR_ICCM_SEG_A_BASE + 4)
#define SCR_ICCM_SEG_A_SNDSTAT_E_LO                 (SCR_ICCM_SEG_A_BASE + 0x3F0)
#define SCR_ICCM_SEG_A_SNDSTAT_E_HI                 (SCR_ICCM_SEG_A_BASE + 0x3F4)
#define SCR_ICCM_SEG_A_SNDSTAT_0_LO                 (SCR_ICCM_SEG_A_BASE + 0x400)
#define SCR_ICCM_SEG_A_SNDSTAT_0_HI                 (SCR_ICCM_SEG_A_BASE + 0x404)
#define SCR_ICCM_SEG_A_SNDSTAT_N_LO(peer_id)        (SCR_ICCM_SEG_A_SNDSTAT_0_LO + (peer_id) * 16)
#define SCR_ICCM_SEG_A_SNDSTAT_N_HI(peer_id)        (SCR_ICCM_SEG_A_SNDSTAT_0_HI + (peer_id) * 16)
#define SCR_ICCM_SEG_A_BUFWRITE_0                   (SCR_ICCM_SEG_A_BASE + 0xC00)
#define SCR_ICCM_SEG_A_BUFWRITE_N(peer_id)          (SCR_ICCM_SEG_A_BUFWRITE_0 + (peer_id) * 4)

/* SEGMENT B */
#define SCR_ICCM_SEG_B_BASE                         (PLF_ICCM_BASE + 0x1000)
#define SCR_ICCM_SEG_B_ICCM_VERSION                 (SCR_ICCM_SEG_B_BASE + 0x0)
#define SCR_ICCM_SEG_B_ICCM_PEERS                   (SCR_ICCM_SEG_B_BASE + 0x4)
#define SCR_ICCM_SEG_B_ICCM_CONTROL                 (SCR_ICCM_SEG_B_BASE + 0x8)
#define SCR_ICCM_SEG_B_ICCM_CLEAR_LO                (SCR_ICCM_SEG_B_BASE + 0x10)
#define SCR_ICCM_SEG_B_ICCM_CLEAR_HI                (SCR_ICCM_SEG_B_BASE + 0x14)

#define SCR_ICCM_BUFSTATUS_FULL_BIT                 (1 << 0)

#define SCR_ICCM_BUFSTATUS_SENDERID_OFFS            (16)
#define SCR_ICCM_BUFSTATUS_SENDERID_MASK            (0xFF << SCR_ICCM_BUFSTATUS_SENDERID_OFFS)

#define SCR_ICCM_BUFSTATUS_OWNID_OFFS               (24)
#define SCR_ICCM_BUFSTATUS_OWNID_MASK               (0xFF << SCR_ICCM_BUFSTATUS_OWNID_OFFS)

#define SCR_ICCM_ICCM_CONTROL_EWE_BIT               (1 << 0)
#define SCR_ICCM_ICCM_CONTROL_ERE_BIT               (1 << 1)
#define SCR_ICCM_ICCM_CONTROL_EEWF_BIT              (1 << 2)
#define SCR_ICCM_ICCM_CONTROL_EIWF_BIT              (1 << 3)

/******************************************************************************
 *                              Types section
 */
typedef uint8_t iccm_peer_id;

typedef struct iccm_data_desc {
	uint32_t data;
	iccm_peer_id sender_id;
} iccm_data_desc_t;

/******************************************************************************
 *                              Implementation section
 */
#ifdef CONFIG_SMP

/**
 * \brief Gets value of ICCM_BUFSTATUS register
 *
 * \return value of ICCM_BUFSTATUS register
 */
static inline uint32_t iccm_get_bufstatus(void)
{
	return *((volatile uint32_t *) SCR_ICCM_SEG_A_BUFSTATUS);
}

/**
 * \brief Returns Peer ID of a local core
 *
 * \return Peer ID of a local core
 */
static inline iccm_peer_id iccm_get_own_peer_id(void)
{
	uint32_t bufstatus = iccm_get_bufstatus();

	return (iccm_peer_id) ((bufstatus & SCR_ICCM_BUFSTATUS_OWNID_MASK) >>
		SCR_ICCM_BUFSTATUS_OWNID_OFFS);
}

/**
 * \brief Check if local Peer has unread messages in a buffer
 *
 * \return true - if local Peer has unread messages in a buffer;
 *		false - otherwise
 */
#if __ASSERT_ON
static inline bool iccm_is_data_available(void)
{
	uint32_t bufstatus = iccm_get_bufstatus();

	return (bufstatus & SCR_ICCM_BUFSTATUS_FULL_BIT) != 0;
}
#endif /* __ASSERT_ON */

/**
 * \brief Get Peer ID of a sender whos message we are about to read
 *
 * \return Peer ID of a sender
 */
static inline iccm_peer_id iccm_get_sender_id(void)
{
	uint32_t bufstatus = iccm_get_bufstatus();

	return (bufstatus & SCR_ICCM_BUFSTATUS_SENDERID_MASK) >>
		SCR_ICCM_BUFSTATUS_SENDERID_OFFS;
}

/**
 * @brief Send message to a Peer with corresponding ID
 *
 * @param receiver_id Peer ID of a receiver
 * @param msg message
 */
static inline void iccm_send(iccm_peer_id receiver_id, uint32_t msg)
{
	*(volatile uint32_t *) SCR_ICCM_SEG_A_BUFWRITE_N(receiver_id) = msg;
}

/**
 * @brief Read incoming IPI message from a buffer
 *
 * @param IPI message being read
 *
 * @return Incoming data descriptor
 */
static inline iccm_data_desc_t iccm_get_data(void)
{
	iccm_data_desc_t desc;

	desc.data = *(volatile uint32_t *) SCR_ICCM_SEG_A_BUFREAD;
	desc.sender_id = iccm_get_sender_id();

	return desc;
}

/**
 * @brief RISC-V hook to deliver IPI to the set of CPU cores
 *		which will result in z_sched_ipi calls on other HARTs
 *
 * @param cpu_bitmap - mask of CPUs to deliver the IPI
 */
void arch_sched_directed_ipi(uint32_t cpu_bitmap)
{
	unsigned int key = arch_irq_lock();
	uint8_t own_peer_id = iccm_get_own_peer_id();
	uint8_t num_cpus = arch_num_cpus();

	/*
	 * We expect that Peer ID matches with Hart ID for intra-cluster Peers.
	 * In general this could not always be true, but for now RISC-V implementation
	 * (e.g. PLIC) relies on consequent Hart IDs anyway
	 */
	for (uint8_t cpu_id = 0; cpu_id < num_cpus; cpu_id++) {
		uint8_t peer_id = _kernel.cpus[cpu_id].arch.hartid;

		if (peer_id != own_peer_id && ((cpu_bitmap & BIT(cpu_id)) != 0)) {
			iccm_send(peer_id, SCR_ICCM_MSG_SCHEDULE);
		}
	}

	arch_irq_unlock(key);
}

/**
 * @brief RISC-V IPI interrupt handler
 */
void iccm_ipi_handler(const void *unused)
{
	__ASSERT(iccm_is_data_available(), "IPI IRQ misfire");

	iccm_data_desc_t desc =	iccm_get_data();

	__ASSERT(desc.data == SCR_ICCM_MSG_SCHEDULE,
		"Invalid Data %x from Peer %d", desc.data, desc.sender_id);
	z_sched_ipi();

	ARG_UNUSED(desc);
	ARG_UNUSED(unused);
}

#endif /* CONFIG_SMP */
