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

#include <zephyr/kernel.h>

#include <soc.h>
#include "generic_cache.h"

#if IS_ENABLED(CONFIG_SCR_L2_GENERIC_CACHE) || IS_ENABLED(CONFIG_SCR_L3_GENERIC_CACHE)

struct gc_regs {
    uint64_t node_info;
    uint64_t version_id;
    uint64_t cluster_info;

    uint64_t cache_info;
    uint64_t cache_ctrl;
    uint64_t cache_status;

    uint64_t feat_info;
    uint64_t feat_ctrl;
    uint64_t feat_status;

    uint64_t cmd_ctrl;
    uint64_t cmd_addr;
    uint64_t cmd_data[8];
    uint64_t cmd_status;

    uint64_t err_mask;
    uint64_t err_status;
    uint64_t derr_info;
    uint64_t terr_info;

    struct {
        uint64_t pce_ctrl;
        uint64_t pce_cntr;
    }pce[8];

    uint64_t scrub_ctrl;
} __attribute__((aligned(8)));

/**
 * @enum cmd_ctrl_opcodes
 * @brief The command to execute
 */
enum cmd_ctrl_opcodes {
    CMD_CTRL_OPCODE_NOP = 0, // No operation

    /** Invalidate cache, and write back Dirty lines downstream.
     * This is equivalent to receiving CleanInvalid transaction
     * for all cached lines. */
    CMD_CTRL_OPCODE_FLUSH_ALL,

    /** Invalidate cache, and discard Dirty lines. This is equivalent
     * to receiving MakeInvalid transaction for all cached lines. */
    CMD_CTRL_OPCODE_INVALID_ALL,

    /** Write Dirty lines downstream while keeping them cached.
     * This is equivalent to receiving CleanShared transaction
     * for all cached lines. */
    CMD_CTRL_OPCODE_CLEAN_ALL,

    /** In case of hit, load in CMD_STATUS the content of the Tag-RAM cell
     * that matches the address given in CMD_ADDR, and in CMD_DATA its
     * cache line data, and in CMD_CTRL its set/way */
    CMD_CTRL_OPCODE_LOAD_BY_ADDR,

    /** When line is in Valid state, load in CMD_STATUS the content
     * of the Tag-RAM cell at the index/way given in CMD_CTRL, and in CMD_DATA
     * its cache line data, and in CMD_ADDR its address. */
    CMD_CTRL_OPCODE_LOAD_BY_SET_WAY,

    /** Execute a memory barrier for all transactions received.
     * The DMB command completes when the effects of requests received before
     * the command are ordered before the effects of requests received
     * after the DMB command. */
    CMD_CTRL_OPCODE_DMB,

    /** Execute a synchronization barrier for all transactions received,
     * and selfgenerated operations.
     * The DSB command completes when the effects and
     * side-effects of requests and commands received before the DSB command
     * have completed. The requests received after the command are ordered after
     * its completion. That is, their effects and side-effects are ordered after
     * the effects and side-effects of requests and commands received
     * before the DSB command. */
    GC_CMD_OPCODE_DSB
};

/**
 * @def CACHE_STATUS_CACHE_ENABLE
 * @def CACHE_STATUS_COHERENCY_ENABLE
 * @brief This register reflects the changes in CACHE_CTRL. Each bit in
 * CACHE_STATUS equals the corresponding one in CACHE_CTRL when the
 * relative effects are observed by all transactions.
 */
#define CACHE_STATUS_CACHE_ENABLE (1ULL << 32)
#define CACHE_STATUS_COHERENCY_ENABLE (1ULL << 24)

/**
 * @def CMD_STATUS_DONE
 * @brief When asserted, no command is executing, and the values in
 * CMD_STATUS register are meaningful. When deasserted, a command
 * is executing, and the values in CMD_CTRL, CMD_ADDR, CMD_DATA
 * and CMD_STATUS registers are meaningless.
 */
#define CMD_STATUS_DONE (1ULL << 0)

/**
 * @def CLUSTER_INFO_BANK_NUM
 * @brief In Home mode, the number of Home Nodes on the
 * interconnect. In proxy mode, the number of cache banks.
 */
#define CLUSTER_INFO_BANK_NUM_MASK 0xFFFF
#define CLUSTER_INFO_BANK_NUM_OFFSET 16

/**
 * @def NODE_INFO_TYPE
 * @brief The component type at this address, that is, the GC.
 */
#define NODE_INFO_TYPE_MASK 0xFFFF

/**
 * @struct gc_description
 * @brief description of Generic Cache block
 *
 */
struct gc_description {
    /**
     * @brief Base address of the generic cache registers.
     */
    struct gc_regs *base_addr;

    /**
      * @brief Indicates whether the cache is private.
      *
      * If true, the cache is private to a specific HART.
      * If false, it is shared among multiple HARTs.
      */
    bool is_private;

    /**
     * @brief Indicates whether this is the last level in the cache hierarchy.
     *
     * If true, this cache block is the final level.
     * If false, there are additional levels above this one.
     */
    bool is_last_level;
};

/**
 * @brief Detail description of each Generic Cache unit
 */
struct gc_description gc_desc[] = {
        { /// GC_L1
                .base_addr = (struct gc_regs *)0
        },

        { /// GC_L1D
                .base_addr = (struct gc_regs *)0
        },

        { /// GC_L1I
                .base_addr = (struct gc_regs *)0,
        },

        { /// GC_L2
#if IS_ENABLED(CONFIG_SCR_L2_GENERIC_CACHE)
                .base_addr = (struct gc_regs *)(PLF_L2CTL_BASE),
#else
                .base_addr = 0,
#endif
                .is_private = true,
                .is_last_level = false
        },

        { /// GC_L3
#if IS_ENABLED(CONFIG_SCR_L3_GENERIC_CACHE)
                .base_addr = (struct gc_regs *)(PLF_L3CTL_BASE),
#else
                .base_addr = 0,
#endif
                .is_private = false,
                .is_last_level = true
        },

        { /// GC_L4
                .base_addr = (struct gc_regs *)0,
                .is_private = false,
                .is_last_level = true
        },
};

static inline volatile struct gc_regs *
common_regs(enum gc_type gc_type)
{
    struct gc_regs *regs = gc_desc[gc_type].base_addr;
    return regs + 0;
}

static inline volatile struct gc_regs *
bank_regs(enum gc_type gc_type, int bank_num)
{
    struct gc_description *desc = gc_desc + gc_type;
    struct gc_regs *regs = desc->base_addr;
    if (desc->is_private)
        return regs;

    return (struct gc_regs *)((char *)regs + (0x200 * (bank_num + 1)));
}

static unsigned gc_banks_num(enum gc_type gc_type)
{
    volatile struct gc_regs *regs = common_regs(gc_type);
    return (regs->cluster_info >> CLUSTER_INFO_BANK_NUM_OFFSET) &
                CLUSTER_INFO_BANK_NUM_MASK;
}

static void gc_enable(enum gc_type gc_type)
{
    int bn, i;
    volatile struct gc_regs *regs = common_regs(gc_type);
    uint64_t cache_en_flags = CACHE_STATUS_CACHE_ENABLE;
    if (!gc_desc[gc_type].is_last_level)
        cache_en_flags |= CACHE_STATUS_COHERENCY_ENABLE;

    regs->cache_ctrl |= cache_en_flags;
    bn = gc_banks_num(gc_type);
    for (i = 0; i < bn; i++) {
        volatile struct gc_regs *b_regs = bank_regs(gc_type, i);
        while((b_regs->cache_status & cache_en_flags) != cache_en_flags);
    }
}

int gc_init(enum gc_type gc_type)
{
    volatile struct gc_regs *regs = common_regs(gc_type);
    uint16_t node_id_vals[] = {
            0x0,  // GC_L1
            0x1,  // GC_L1D
            0x2,  // GC_L1I
            0x10, // GC_L2
            0x20, // GC_L3
            0x30  // GC_L4
    };
    int bn, i;

    if ((regs->node_info & NODE_INFO_TYPE_MASK) != node_id_vals[gc_type])
        return -1;

    /*
     * Invalidate all banks, this CMO is triggered without spinlock since
     * at this point the caches are not yet enabled and the atomic operations
     * couldn't be used
     */
    regs->cmd_ctrl = CMD_CTRL_OPCODE_INVALID_ALL;
    bn = gc_banks_num(gc_type);
    for (i = 0; i < bn; i++) {
        volatile struct gc_regs *b_regs = bank_regs(gc_type, i);
        while(!(b_regs->cmd_status & CMD_STATUS_DONE));
    }

    gc_enable(gc_type);
    return 0;
}

#endif /* IS_ENABLED(CONFIG_SCR_L2_GENERIC_CACHE) || IS_ENABLED(CONFIG_SCR_L3_GENERIC_CACHE) */
