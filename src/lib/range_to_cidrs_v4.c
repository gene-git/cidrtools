/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * Helper for IPv4 range => cidrs.
 * non-public
 */
int range_to_cidrs_v4(uint32_t start, uint32_t end, CtCidrs *cidrs) {
    unsigned int trailing_zeros = 0;
    uint64_t diff = 0U;
    CtCidr current_cidr = {};

    while (start <= end) {
        if (start != 0U) {
            trailing_zeros = (unsigned int)__builtin_ctz(start);
        } else {
            trailing_zeros = 32U; 
        }

        diff = (uint64_t)end - start + 1U;
        while (trailing_zeros > 0U && ((uint64_t)1 << trailing_zeros) > diff) {
            trailing_zeros--;
        }

        current_cidr.prefix = (uint8_t)(32U - trailing_zeros);

        current_cidr.addr.family = AF_INET;
        current_cidr.addr.addr.v4.s_addr = htonl(start);
        if (!ct_add_cidr_to_cidrs(cidrs, &current_cidr)) {
            return -1;
        }

        if (start == UINT32_MAX || trailing_zeros == 32U) {
            break;
        }
        start += (1U << trailing_zeros);
    }
    return 0;
}

