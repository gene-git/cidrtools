/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_SPLIT_COUNT 65536ULL

/**
 * Increments an IPv6 address using high-performance 32-bit native arithmetic blocks.
 * Replaces costly byte-loop array adjustments with direct CPU register bit strides.
 */
static void add_v6_bit_step_32(uint32_t *addr32, uint8_t prefix_len) {
    unsigned int shift_bit = 0U;
    int block_idx = 0U;
    unsigned int bit_idx = 0U;
    uint64_t carry = 0U;
    uint32_t host_val = 0U;
    int i = 0U;

    shift_bit = 128U - prefix_len;
    block_idx = (int)(shift_bit / 32U);
    bit_idx = shift_bit % 32U;

    /*
     * carry begins at the target prefix stride slot
     */
    carry = (uint64_t)1U << bit_idx;

    /*
     * Cascade the addition carry right-to-left across 32-bit register double-words
     */
    for (i = 3 - block_idx; i >= 0; i--) {
        host_val = ntohl(addr32[i]);
        carry += host_val;
        addr32[i] = htonl((uint32_t)(carry & 0xFFFFFFFFU));
        carry >>= 32U;
        if (carry == 0ULL) {
            break;
        }
    }
}

/**
 * Splits a cidr into a list of smaller subnets of with prefix lengths 'prefix'.
 * The provided prefix must be larger then cidr->prefix (obviously).
 *
 * Caller is responsible for freeing the memory of the resulting list of cidrs.
 * Both the result->blocks as well as the CtCidrs structure itself.
 *
 * :param cidr: The cidr to split into smaller subnets.
 * :param prefix: The prefix to use.
 *
 * :returns: A pointer to a new heap-allocated CtCidrs holding the array of cidrs. 
 *           Or a nullptr on failure.
 */
CtCidrs *ct_subnets_split(const CtCidr *cidr, uint8_t prefix) {
    uint8_t bit_diff = 0U;
    size_t total_subnets = 0U;
    uint64_t count_check = 0U;
    CtCidrs *result = nullptr;
    CtCidr clean_base = {};
    uint32_t current_ip = 0;
    uint32_t step_size = 0;
    size_t i = 0;
    CtCidr *block = nullptr;
    CtCidr *prev_block = nullptr;

    if (!cidr) {
        return nullptr;
    }

    if (prefix < cidr->prefix) {
        return nullptr; 
    }
    
    if (cidr->addr.family == AF_INET && prefix > 32U) {
        return nullptr;
    }
    
    if (cidr->addr.family == AF_INET6 && prefix > 128U) {
        return nullptr;
    }

    bit_diff = (uint8_t)(prefix - cidr->prefix);
    total_subnets = 1U;
    
    if (bit_diff > 0U) {
        if (bit_diff >= 64U) {
            return nullptr; 
        }
        count_check = (uint64_t)1U << bit_diff;
        if (count_check > MAX_SPLIT_COUNT) {
            return nullptr; 
        }
        total_subnets = (size_t)count_check;
    }

    result = malloc(sizeof(CtCidrs));
    if (!result) {
        return nullptr;
    }

    result->blocks = malloc(total_subnets * sizeof(CtCidr));
    if (!result->blocks) {
        free(result);
        return nullptr;
    }
    result->count = total_subnets;

    clean_base = *cidr;
    if (ct_cidr_fix_host_bits(&clean_base) != 0) {
        free(result->blocks);
        free(result);
        return nullptr;
    }

    switch (cidr->addr.family) {
        case AF_INET:
            current_ip = ntohl(clean_base.addr.addr.v4.s_addr);
            step_size = (prefix == 32U) ? 1U : (1U << (32U - prefix));

            for (i = 0; i < total_subnets; i++) {
                block = &result->blocks[i];

                block->prefix = prefix;
                block->addr.family = AF_INET;
                block->addr.addr.v4.s_addr = htonl(current_ip);
                current_ip += step_size;
            }
            break;

        case AF_INET6:
            /*
             * Prime initial 32-bit block
             */
            for (i = 0; i < 4U; i++) {
                result->blocks[0].addr.addr.v6.s6_addr32[i] = clean_base.addr.addr.v6.s6_addr32[i];
            }
            result->blocks[0].addr.family = AF_INET6;
            result->blocks[0].prefix = prefix;

            for (i = 1; i < total_subnets; i++) {
                prev_block = &result->blocks[i - 1];
                block = &result->blocks[i];

                block->prefix = prefix;
                block->addr.family = AF_INET6;
                
                /*
                 * Duplicate the previous block
                 */
                block->addr.addr.v6.s6_addr32[0] = prev_block->addr.addr.v6.s6_addr32[0];
                block->addr.addr.v6.s6_addr32[1] = prev_block->addr.addr.v6.s6_addr32[1];
                block->addr.addr.v6.s6_addr32[2] = prev_block->addr.addr.v6.s6_addr32[2];
                block->addr.addr.v6.s6_addr32[3] = prev_block->addr.addr.v6.s6_addr32[3];

                /*
                 * Increment the data register values forward in-place
                 */
                add_v6_bit_step_32(block->addr.addr.v6.s6_addr32, prefix);
            }
            break;

        default:
            free(result->blocks);
            free(result);
            return nullptr;
    }

    return result;
}

