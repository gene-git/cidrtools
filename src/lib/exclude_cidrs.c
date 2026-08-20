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
 * helper to push an individual block into the array
 * Returns false on memory error, otherwise true.
 */
static bool push_block(CtCidrs *list, const CtCidr *block) {
    size_t new_cap = list->count + 1U;

    CtCidr *tmp = realloc(list->blocks, new_cap * sizeof(CtCidr));
    if (!tmp) {
        return false;
    }

    list->blocks = tmp;
    list->blocks[list->count] = *block;
    list->count = new_cap;
    return true;
}

/*
 * Subtracts 'ex' from 'broad' and appends the remaining fragments to 'out_list'.
 * If they do not intersect, 'broad' is appended to out_list unmodified.
 *
 * Returns false if memory error (see push_block) otherwise returns true.
 */
static bool cidr_exclude_single(const CtCidr *broad, const CtCidr *ex, CtCidrs *out_list) {
    CtCidr current_fragment = {};
    uint8_t target_prefix = 0U;

    /*
     * If families mismatch or they don't intersect, preserve the broad block
     */
    if (!ct_cidr_contains_cidr(broad, ex)) {
        return push_block(out_list, broad);
    }

    /*
     * If 'ex' matches or is wider than 'broad', the whole block is ignored (nothing remains)
     */
    if (ex->prefix <= broad->prefix) {
        return true; 
    }

    /*
     * Subnet splitting : work down bit-by-bit from broad->prefix to ex->prefix
     */
    current_fragment = *broad;
    target_prefix = ex->prefix;

    while (current_fragment.prefix < target_prefix) {
        /*
         * Go down by one bit
         */
        uint8_t next_prefix = (uint8_t)(current_fragment.prefix + 1U);
        
        CtCidr left_sub = current_fragment;
        left_sub.prefix = next_prefix;
        
        CtCidr right_sub = current_fragment;
        right_sub.prefix = next_prefix;

        switch(broad->addr.family) {
            case AF_INET:
                uint32_t orig_ip = ntohl(current_fragment.addr.addr.v4.s_addr);

                /* 
                 * Flip the bit at the new prefix boundary to get the right-side child address
                 */
                uint32_t right_ip = orig_ip | (1U << (32U - next_prefix));
                right_sub.addr.addr.v4.s_addr = htonl(right_ip);
                break;

            case AF_INET6:
                /*
                 * Use 32-bit blocks for performance.
                 */
                uint8_t  block_idx = (uint8_t)((next_prefix - 1U) / 32U);
                uint8_t  bit_idx = (uint8_t)(31U - ((next_prefix - 1U) % 32U));

                uint32_t host_chunk = ntohl(current_fragment.addr.addr.v6.s6_addr32[block_idx]);
            
                host_chunk |= (1U << bit_idx);
                right_sub.addr.addr.v6.s6_addr32[block_idx] = htonl(host_chunk);
                break;

            default:
                break;
        }

        /*
         * Find which side of the child split has the exclusion target
         */
        if (ct_cidr_contains_cidr(&left_sub, ex)) {
            /*
             * Exclusion target sits in left split - save right split and keep processing left
             */
            if (!push_block(out_list, &right_sub)) {
                return false;
            }
            current_fragment = left_sub;

        } else {
            /*
             * Exclusion target sits in right split - save the left split and keep processing right
             */
            if (!push_block(out_list, &left_sub)) {
                return false;
            }
            current_fragment = right_sub;
        }
    }

    return true;
}

/**
 * Remove a set of cidr blocks from an array of cidrs.
 *
 * :param all: The list of cidrs to be modified in place. The modifed
 *             array will have all cidrs in the *excluded* list removed.
 * :param excludewd: The list of cidrs to be exluded from the full list.
 *
 * :returns: 0 on success, -1 otherwise.
 */
int ct_exclude_cidrs(CtCidrs *all, CtCidrs *excluded) {

    if (!all) {
        return 0;
    }

    if (!excluded || excluded->count == 0 || !excluded->blocks) {
        /*
         * No exclusions - nothing to do
         */
        return 0;
    }

    /* 
     * Compact excluded lookups on this have no overlaps and are ordered.
     */
    if (ct_compact(excluded) < 0) {
        return -1;
    }

    /*
     *  Remove each exclusion from the 'all' pool of cidrs.
     */
    for (size_t ex_idx = 0; ex_idx < excluded->count; ex_idx++) {
        CtCidrs round_output = { .blocks = nullptr, .count = 0 };

        for (size_t all_idx = 0; all_idx < all->count; all_idx++) {
            if (!cidr_exclude_single(&all->blocks[all_idx], &excluded->blocks[ex_idx], &round_output)) {
                /*
                 * memory error via push_block
                 */
                free(round_output.blocks);
                return -1; 
            }
        }

        /*
         * Replace the original 'all' array with new split fragments list
         */
        free(all->blocks);
        all->blocks = round_output.blocks;
        all->count = round_output.count;

        if (all->count == 0) {
            /*
             *  Entire pool space was excluded, nothing left to do
             */
            all->blocks = nullptr;
            break; 
        }
    }

    /*
     * Make sure to compact the result.
     */
    if (ct_compact(all) < 0) {
        return -1;
    }
    return 0;
}

