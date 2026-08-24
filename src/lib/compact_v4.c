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

/*
 * IPv4 Helpers
 */
static bool try_merge_adjacent_v4(const CtCidr *a, const CtCidr *b, CtCidr *merged) {
    if (!a || !b || !merged) {
        return false;
    }

    /*
     * Blocks must have identical prefix lengths and cannot be default route (prefix == 0)
     */
    //if (a->prefix != b->prefix || a->prefix == 0U || a->prefix > 32U) {
    if (a->prefix != b->prefix || a->prefix < 1U || a->prefix > 32U) {
        return false;
    }

    uint32_t ip_a = ntohl(a->addr.addr.v4.s_addr);
    uint32_t ip_b = ntohl(b->addr.addr.v4.s_addr);

    /*
     * Check adjacent binary bits.
     * To merge into a parent prefix (prefix - 1), the two subnets MUST differ
     * ONLY on the exact bit of their current prefix length.
     */
    uint32_t buddy_bit = 1U << (32U - a->prefix);

    /*
     * Check if toggling that single bit turns IP A into IP B
     */
    if ((ip_a ^ buddy_bit) == ip_b) {
        uint8_t target_prefix = (uint8_t)(a->prefix - 1U);
        uint32_t mask = (target_prefix == 0U) ? 0x00000000U : (0xFFFFFFFFU << (32U - target_prefix));

        merged->addr.family = AF_INET;
        merged->addr.addr.v4.s_addr = htonl(ip_a & mask);
        merged->prefix = target_prefix;
        return true;
    }

    return false;
}


/*
 * Compact a list of ``IPv4`` cidr blocks to the smallest number of cidr blocks.
 * Note - internal function - caller is compact() which has already
 * checked for sensible input (cidrs, cidrs->count > 1 etc)
 */
void compact_v4(CtCidrs *cidrs) {
    /*
     * Safety for empty inputs
    if (!cidrs || cidrs->count <= 1 || !cidrs->blocks) {
        return;
    }
     */
    
    bool modified = true;
    bool needs_sort = true; 
    qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), ct_cidr_sort_compare);
    needs_sort = false;

    /*
     * Special case 0.0.0.0/0
     */
    if (cidrs->blocks[0].prefix == 0) {
        cidrs->count = 1U;
        return;
    }

    while (modified) {
        /*
         *  Only re-sort if order was broken by a previous merge loop
         */
        if (needs_sort) {
            qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), ct_cidr_sort_compare);
            needs_sort = false;
        }

        size_t write_idx = 0;
        modified = false;
        
        for (size_t i = 1; i < cidrs->count; i++) {
            CtCidr *current_stable = &cidrs->blocks[write_idx];
            CtCidr *next_candidate = &cidrs->blocks[i];

            // 1. Check for containment/duplicates
            if (ct_cidr_contains_cidr(current_stable, next_candidate)) {
                modified = true;
                continue;
            }

            // 2. Try merging adjacent blocks
            CtCidr merged_block = {};
            if (try_merge_adjacent_v4(current_stable, next_candidate, &merged_block)) {

                *current_stable = merged_block;
                modified = true;

                // CHECK IF ORDER WAS BROKEN:
                // If this expanded block now out-indexes the element directly behind it,
                // we mark the array as dirty to trigger a quick qsort repair on the next pass.
                if (write_idx > 0) {
                    if (ct_cidr_sort_compare(&cidrs->blocks[write_idx - 1], current_stable) > 0) {
                        needs_sort = true;
                    }
                }
                continue;
            }

            // 3. No merge possible; preserve next_candidate by moving it up
            write_idx++;
            if (write_idx != i) {
                cidrs->blocks[write_idx] = *next_candidate;
            }
        }
        cidrs->count = write_idx + 1;
    }
}

