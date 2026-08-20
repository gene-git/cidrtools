/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>

/*
 * internal comparison function for IPv4 and IPv6
 */
int cidr_sort_compare(const void *a, const void *b) {
    const CtCidr *block_a = (const CtCidr *)a;
    const CtCidr *block_b = (const CtCidr *)b;
    uint32_t ip_a = 0;
    uint32_t ip_b = 0;
    int cmp = 0;

    if (a == b) {
        return 0;
    }

    /*
     * Sort by family first (IPv4 ahead of IPv6)
     */
    if (block_a->addr.family != block_b->addr.family) {
        return (block_a->addr.family < block_b->addr.family) ? -1 : 1;
    }

    switch (block_a->addr.family) {
        case AF_INET:
            ip_a = ntohl(block_a->addr.addr.v4.s_addr);
            ip_b = ntohl(block_b->addr.addr.v4.s_addr);

            if (ip_a != ip_b) {
                return (ip_a < ip_b) ? -1 : 1;
            }

            /*
             * Tie breaker
             * - Wider subnets (smaller prefix numbers, e.g., /8) sort BEFORE smaller subnets
             */
            if (block_a->prefix < block_b->prefix) {
                return -1;
            }

            if (block_a->prefix > block_b->prefix) {
                return 1;
            }

            /* 
             * match 
             */
            return 0; 

        case AF_INET6:
            cmp = memcmp(block_a->addr.addr.v6.s6_addr, block_b->addr.addr.v6.s6_addr, 16);
            if (cmp != 0) {
                return cmp;
            }

            /*
             * Tie breaker
             */
            if (block_a->prefix < block_b->prefix) {
                return -1;
            }

            if (block_a->prefix > block_b->prefix) {
                return 1;
            }

            /*
             * matched
             */
            return 0;

        default:
            return 0;
    }
}

#ifdef UNUSEDOLD
static int cidr_sort_compare(const void *a, const void *b) {
    const CtCidr *block_a = (const CtCidr *)a;
    const CtCidr *block_b = (const CtCidr *)b;
    uint32_t ip_a = 0;
    uint32_t ip_b = 0;
    int cmp = 0;

    /*
     *  Sort by family first (IPv4 ahead of IPv6)
     */
    if (block_a->addr.family != block_b->addr.family) {
        return (block_a->addr.family < block_b->addr.family) ? -1 : 1;
    }

    switch (block_a->addr.family) {
        case AF_INET:
            ip_a = ntohl(block_a->addr.addr.v4.s_addr);
            ip_b = ntohl(block_b->addr.addr.v4.s_addr);

            if (ip_a != ip_b) {
                return (ip_a < ip_b) ? -1 : 1;
            }

            /*
             * Tie-breaker: 
             * - IPs are identical, wider prefixes (smaller numbers, e.g. /16) sort first
             */
            return (block_a->prefix < block_b->prefix) ? -1 : (block_a->prefix > block_b->prefix);

        case AF_INET6:
            cmp = memcmp(block_a->addr.addr.v6.s6_addr, block_b->addr.addr.v6.s6_addr, 16);
            if (cmp != 0) {
                return cmp;
            }

            /*
             * Tie-breaker
             */
            return (block_a->prefix < block_b->prefix) ? -1 : (block_a->prefix > block_b->prefix);

        default:
            return 0;
    }
}
#endif

