/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Compact a list of cidr blocks to the smallest number of cidr blocks.
 *
 * Does in place compacting of the CtCidrs. If cidr blocks can be merged
 * into larger blocks (smaller prefixes) then the number of blocks is reduced.
 * If the blocks are able to be compacted, then cidrs->count will be reduced
 * and the memory cidrs->blocks adjusted acordingly.
 *
 * cidr blocks can be mixed ``IPV4`` and ``IPv6`` families.
 *
 * :param cidrs: The list of cidr_blocks to be compacted
 *
 * :returns: -1 on error, otherwise 0.
 */
int ct_compact(CtCidrs *cidrs) {
    size_t count_orig = 0;

    if (!cidrs || cidrs->count <= 1 || !cidrs->blocks) {
        return 0;
    }

    /*
     * Family split
     */
    count_orig = cidrs->count;
    CtCidrs cidrs_v4 = {};
    CtCidrs cidrs_v6 = {};

    if (ct_split_by_family(cidrs, &cidrs_v4, &cidrs_v6) != 0) {
        return -1;
    }

    if (cidrs_v4.count + cidrs_v6.count < 2U) {
        return -1;
    }

    if (cidrs_v4.count > 1) {
        compact_v4(&cidrs_v4);
    }

    if (cidrs_v6.count > 1) {
        compact_v6(&cidrs_v6);
    }
    
    /*
     * Put back together
     */
    for (size_t i = 0; i < cidrs_v4.count; i++) {
        cidrs->blocks[i] = cidrs_v4.blocks[i];
    }

    size_t count_0 = cidrs_v4.count;
    for (size_t i = 0; i < cidrs_v6.count; i++) {
        cidrs->blocks[count_0 + i] = cidrs_v6.blocks[i];
    }

    /*
     * Resize and free up
     */
    count_0 = cidrs_v4.count + cidrs_v6.count;
    if (count_0 != count_orig) {
        if (!ct_allocate_cidrs(count_0, cidrs)) {
            return -1;
        }
        //ptr = realloc(cidrs->blocks, cidrs->count * sizeof(CtCidr));
        //if (!ptr) {
        //    return -1;
        //}
        //cidrs->blocks = (CtCidr *)ptr;
    }
    ct_free_cidrs(&cidrs_v4);
    ct_free_cidrs(&cidrs_v6);
    return 0;
}

