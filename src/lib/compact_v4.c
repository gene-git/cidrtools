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
#ifdef NOT_USED
static bool _unused_try_merge_adjacent_v4(const CtCidr *a, const CtCidr *b, CtCidr *merged) {
    uint8_t target_prefix = 0U;
    uint32_t ip_a = 0U;
    uint32_t ip_b = 0U;
    uint32_t mask = 0U;

    if (a->prefix != b->prefix || a->prefix <= 0U || a->prefix > 32U) {
        return false;
    }

    target_prefix = (uint8_t)(a->prefix - 1U);
    ip_a = ntohl(a->addr.addr.v4.s_addr);
    ip_b = ntohl(b->addr.addr.v4.s_addr);
    mask = (target_prefix == 0U) ? 0x00000000U : (0xFFFFFFFFU << (32U - target_prefix));

    if ((ip_a & mask) == (ip_b & mask)) {
        merged->addr.family = AF_INET;
        merged->addr.addr.v4.s_addr = htonl(ip_a & mask);
        merged->prefix = target_prefix;
        return true;
    }
    return false;
}
#endif

static bool try_merge_adjacent_v4(const CtCidr *a, const CtCidr *b, CtCidr *merged) {
    if (!a || !b || !merged) {
        return false;
    }

    /*
     * Blocks must have identical prefix lengths and cannot be default route (prefix == 0)
     */
    if (a->prefix != b->prefix || a->prefix == 0U || a->prefix > 32U) {
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
#ifdef UNUSED_CODE
void old_compact_v4(CtCidrs *cidrs) {
    bool modified = true;

    //qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), cidr_compare_v4);
    qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), cidr_sort_compare);

    while (modified) {
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
            if (try_merge_adjacent_v4(current_stable, next_candidate, &merged_block)) {
                *current_stable = merged_block;
                modified = true;
                continue;
            }

            write_idx++;
            cidrs->blocks[write_idx] = *next_candidate;
        }
        cidrs->count = write_idx + 1;
    }
}
#endif

/*
 * Compact a list of ``IPv4`` cidr blocks to the smallest number of cidr blocks.
 */
void compact_v4(CtCidrs *cidrs) {
    /*
     * Safety for empty inputs
     */
    if (!cidrs || cidrs->count <= 1 || !cidrs->blocks) {
        return;
    }
    
    /*
     * Always sort on first pass 
     */
    bool modified = true;
    bool needs_sort = true; 

    while (modified) {
        /*
         *  Only re-sort if order was broken by a previous merge loop
         */
        if (needs_sort) {
            qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), cidr_sort_compare);
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
                // CATCH THE MUTATION IN THE ACT
                /*
                if (merged_block.prefix == 2 && merged_block.addr.addr.v4.s_addr == htonl(0x80000000)) {
                    char *str_a = ct_cidr_to_str(current_stable);
                    char *str_b = ct_cidr_to_str(next_candidate);
                    printf("CRITICAL: compact_v4 merged '%s' and '%s' into '128.0.0.0/2'!\n", str_a, str_b);
                    free(str_a);
                    free(str_b);
                }
                */

                *current_stable = merged_block;
                modified = true;

                // CHECK IF ORDER WAS BROKEN:
                // If this expanded block now out-indexes the element directly behind it,
                // we mark the array as dirty to trigger a quick qsort repair on the next pass.
                if (write_idx > 0) {
                    if (cidr_sort_compare(&cidrs->blocks[write_idx - 1], current_stable) > 0) {
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

