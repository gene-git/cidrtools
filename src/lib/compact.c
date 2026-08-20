/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Compact a list of cidr blocks to the smallest number of cidr blocks.
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
int ct_compact(CtCidrs *cidrs) {
    size_t count_orig = 0;
    void *ptr = nullptr;

    if (!cidrs || cidrs->count == 0 || !cidrs->blocks) {
        return 0;
    }

    count_orig = cidrs->count;

    switch (cidrs->blocks[0].addr.family) {
        case AF_INET:
            compact_v4(cidrs);
            break;

        case AF_INET6:
            compact_v6(cidrs);
            break;

        default:
            return -1;
    }

    if (cidrs->count != count_orig) {
        ptr = realloc(cidrs->blocks, cidrs->count * sizeof(CtCidr));
        if (!ptr) {
            return -1;
        }
        cidrs->blocks = (CtCidr *)ptr;
    }

    return 0;
}

