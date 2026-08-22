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

/**
 * Neighboring merging blocks uses 32-bit block layout.
 * Runs 4x fewer loop updates than older code using byte-shifts.
 */
static bool try_merge_adjacent_v6(const CtCidr *a, const CtCidr *b, CtCidr *merged) {
    uint8_t target_prefix = 0U;
    uint8_t full_blocks = 0U;
    uint8_t remaining_bits = 0U;
    uint8_t i = 0;
    bool match = true;

    if (a->prefix != b->prefix || a->prefix <= 8U || a->prefix > 128U) {
        return false;
    }

    target_prefix = (uint8_t)(a->prefix - 1U);
    full_blocks = target_prefix / 32U;
    remaining_bits = target_prefix % 32U;

    /*
     *  Compare the full host only 32-bit blocks (in Network Byte Order)
     */
    for (i = 0; i < full_blocks; i++) {
        if (a->addr.addr.v6.s6_addr32[i] != b->addr.addr.v6.s6_addr32[i]) {
            return false;
        }
    }

    /*
     * The border block (mixed netowrk and host bits). 
     * Check those partial block boundaries
     */
    if (full_blocks < 4) {

        uint32_t a_block = ntohl(a->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t b_block = ntohl(b->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t mask = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));

        if ((a_block & mask) != (b_block & mask)) {
            match = false;
        }
    }

    if (match) {
        merged->addr.family = AF_INET6;
        merged->prefix = target_prefix;
        
        /*
         * Populate the merged cidr
         */
        for (i = 0; i < 4; i++) {
            if (i < full_blocks) {
                merged->addr.addr.v6.s6_addr32[i] = a->addr.addr.v6.s6_addr32[i];

            } else if (i == full_blocks) {
                uint32_t host_a = ntohl(a->addr.addr.v6.s6_addr32[full_blocks]);
                uint32_t mask = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));
                merged->addr.addr.v6.s6_addr32[i] = htonl(host_a & mask);

            } else {
                merged->addr.addr.v6.s6_addr32[i] = 0x00000000U;
            }
        }
        return true;
    }
    return false;
}

/*
 * Compact a list of cidr blocks to the smallest number of cidr blocks.
 *
 * Private Helper called from ct_compact()
 *
 * Does in place compacting of the CtCidrs. If cidr blocks can be merged
 * into larger blocks (smaller prefixes) then the number of blocks is reduced.
 * If the blocks are able to be compacted, then cidrs->count will be reduced
 * and the memory cidrs->blocks adjusted acordingly.
 *
 * All the cidr blocks must be the same IP family - either IPv4 or IPv6
 *
 * :param cidrs: The list of cidr_blocks to be compacted
 *
 * :returns: -1 on error, otherwise 0.
 */
void compact_v6(CtCidrs *cidrs) {
    if (!cidrs || cidrs->count <= 1 || !cidrs->blocks) {
        return;
    }

    bool modified = true;
    bool needs_sort = true; // Initial sort is mandatory

    while (modified) {
        // Optimize: Only pay the qsort penalty if the array ordering was actually broken
        if (needs_sort) {
            qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), ct_cidr_sort_compare);
            needs_sort = false;
        }

        size_t write_idx = 0;
        modified = false;

        for (size_t i = 1; i < cidrs->count; i++) {
            CtCidr *current_stable = &cidrs->blocks[write_idx];
            CtCidr *next_candidate = &cidrs->blocks[i];

            if (ct_cidr_contains_cidr(current_stable, next_candidate)) {
                modified = true;
                continue;
            }

            CtCidr merged_block = {};
            if (try_merge_adjacent_v6(current_stable, next_candidate, &merged_block)) {
                *current_stable = merged_block;
                modified = true;

                // CHECK IF ORDER WAS BROKEN:
                // If we aren't at the very first element of the array, verify if our
                // newly expanded block has outindexed the item sitting right behind it.
                if (write_idx > 0) {
                    if (ct_cidr_sort_compare(&cidrs->blocks[write_idx - 1], current_stable) > 0) {
                        needs_sort = true; // Order broken! A resort will be required next pass.
                    }
                }
                continue;
            }

            write_idx++;
            cidrs->blocks[write_idx] = *next_candidate;
        }
        cidrs->count = write_idx + 1;
    }
}

