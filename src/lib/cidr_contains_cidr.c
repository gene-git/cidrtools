/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * Internal helper to check containment for IPv4 CIDR blocks.
 * Designed to satisfy aggressive static linting flags.
 */
static inline bool ct_cidr_contains_cidr_v4(const CtCidr *parent, const CtCidr *target) {
    uint32_t parent_ip = 0U;
    uint32_t target_ip = 0U;
    uint32_t mask_v4 = 0U;

    if (parent->prefix > 32U) {
        return false;
    }

    parent_ip = ntohl(parent->addr.addr.v4.s_addr);
    target_ip = ntohl(target->addr.addr.v4.s_addr);

    /*
     *  bitmask for values 0 through 32
     */
    mask_v4 = (parent->prefix == 0U) ? 0x00000000U : (0xFFFFFFFFU << (32U - parent->prefix));

    return (parent_ip & mask_v4) == (target_ip & mask_v4);
}

/*
 * helper to check if IPv6 target in parent CIDR blocks.
 * Use 32-bit blocks for performance
 * Better than 4x over byte-by-byte masking.
 */
static inline bool ct_cidr_contains_cidr_v6(const CtCidr *parent, const CtCidr *target) {
    uint8_t full_blocks = 0;
    uint8_t remaining_bits = 0;

    if (parent->prefix > 128U) {
        return false;
    }

    full_blocks = parent->prefix / 32U;
    remaining_bits = parent->prefix % 32U;

    /*
     * Fast path: Check fully locked network double-words in native Network Byte Order
     */
    for (uint8_t i = 0; i < full_blocks; i++) {
        if (parent->addr.addr.v6.s6_addr32[i] != target->addr.addr.v6.s6_addr32[i]) {
            return false;
        }
    }

    /*
     *  Boundry block: mask evaluations only if leftover bits remain
     */
    if (full_blocks < 4) {
        uint32_t p_block = ntohl(parent->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t t_block = ntohl(target->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t mask_v6 = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));

        if ((p_block & mask_v6) != (t_block & mask_v6)) {
            return false;
        }
    }

    return true;
}

/**
 * Checks whether one cidr block is contained within another.
 *
 * :param target: The cidr to check 
 * :param parent: The parent cidr.
 *
 * :returns: true if the target network is within the parent network.
 */
bool ct_cidr_contains_cidr(const CtCidr *parent, const CtCidr *target) {
    if (!parent || !target) {
        return false;
    }

    if (parent->addr.family != target->addr.family) {
        return false;
    }

    /*
     * Parent subnet must contain the target. Can be the same or larger subnet.
     */
    if (target->prefix < parent->prefix) {
        return false;
    }

    switch (parent->addr.family) {
        case AF_INET:
            return ct_cidr_contains_cidr_v4(parent, target);

        case AF_INET6:
            return ct_cidr_contains_cidr_v6(parent, target);

        default:
            return false;
    }
}

