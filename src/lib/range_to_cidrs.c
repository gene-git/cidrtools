/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

// -----------------------------------------------------------------------------
// Unified API Entry Point
// -----------------------------------------------------------------------------
/**
 * Create a list of cidr blocks that fully span an IP range.
 *  
 * :param first: The first IP in the range
 * :param last: The last IP in the range 
 * :param cidrs: The list of cidrs blocks that span the requested IP range.
 *  
 * :returns: 0 if success, -1 otherwise.
 *
 */
int ct_range_to_cidrs(const CtAddress *first, const CtAddress *last, CtCidrs *cidrs) {
    uint32_t start_host = 0U;
    uint32_t end_host = 0U;

    if (!first || !last || !cidrs) {
        return -1;
    }

    if (first->family != last->family) {
        return -1; 
    }

    cidrs->blocks = nullptr;
    cidrs->count = 0;

    switch (first->family) {
        case AF_INET:
            start_host = ntohl(first->addr.v4.s_addr);
            end_host = ntohl(last->addr.v4.s_addr);

            if (start_host > end_host) {
                return -1;
            }
            return range_to_cidrs_v4(start_host, end_host, cidrs);

        case AF_INET6:
            if (compare_uint8_t(first->addr.v6.s6_addr, last->addr.v6.s6_addr) > 0) {
                return -1; 
            }
            return range_to_cidrs_v6(first->addr.v6.s6_addr, last->addr.v6.s6_addr, cidrs);

        default:
            return -1;
    }
}

