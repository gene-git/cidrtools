/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdlib.h>
#include <sys/socket.h>


/**
 * Takes a list of cidrs and returns lists of ``IPv4`` and ``IPv6`` cidrs.
 *
 * Caller responsible for freeing the memory using ct_free_cidrs().
 *
 * :param cidrs:  The input list of cidrs
 * :param cidrs_v4: The ``IPv4`` cidrs
 * :param cidrs_v6: The ``IPv6`` cidrs 
 * :returns: 0 on success, otherwise -1
 */
int ct_split_by_family(CtCidrs *cidrs, CtCidrs *cidrs_v4, CtCidrs *cidrs_v6) {

    if (!cidrs || !cidrs_v4 || !cidrs_v6) {
        return -1;
    }

    /*
     * Get mem (or clear if no cidrs on input)
     * - allocate enough for all ipv4 or all ipv6 - then realloc after split
     */
    if (!ct_allocate_cidrs(cidrs->count, cidrs_v4)) {
        return -1;
    }

    if (!ct_allocate_cidrs(cidrs->count, cidrs_v6)) {
        ct_free_cidrs(cidrs_v4);
        return -1;
    }

    if (cidrs->count < 1 || !cidrs->blocks) {
        return 0;
    }

    size_t count_v4 = 0U;
    size_t count_v6 = 0U;

    for (size_t i = 0; i < cidrs->count; i++) {
        switch (cidrs->blocks[i].addr.family) {
            case AF_INET:
                cidrs_v4->blocks[count_v4++] = cidrs->blocks[i];
                break;

            case AF_INET6:
                cidrs_v6->blocks[count_v6++] = cidrs->blocks[i];
                break;

            default:
                break;
        }
    }

    /*
     * Reallocate to what was used.
     */
    if (!ct_allocate_cidrs(count_v4, cidrs_v4)) {
        return -1;
    }

    if (!ct_allocate_cidrs(count_v6, cidrs_v6)) {
        return -1;
    }

    return 0;
}
