/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

enum {
    TWO_TO_32 = 4294967296ULL,
};

/**
 * Returns the number of IPs in a CIDR block.
 *
 * Note that ``IPv6`` will overflow 64 bit integer for any prefix
 * 64 or lower. 
 *
 * :param cidr: The cidr block to examine.
 *
 * :returns: The number of IPs in the block. Primarily useful for IPv4
 *           Since IPv6 ranges are large, SIZE_MAX will be returned if 
 *           the result overflows a 64-bit unsigned integer.
 */
size_t ct_num_ips(const CtCidr *cidr) {
    uint8_t host_bits = 0U;

    if (!cidr) {
        return 0U;
    }

    switch (cidr->addr.family) {
        case AF_INET:
            if (cidr->prefix > 32U) {
                return 0U;
            }
            
            /*
             *  care with prefix == 0 and bit shifts
             */
            if (cidr->prefix == 0U) {
                /*
                 * 2*32
                 */
                return TWO_TO_32;
            }
            
            return (size_t)1U << (32U - cidr->prefix);

        case AF_INET6:
            if (cidr->prefix > 128U) {
                return 0U;
            }
            
            host_bits = (uint8_t)(128U - cidr->prefix);
            
            /*
             *  Care for host bits >= 64 causing 64-bit size_t overflow
             */
            if (host_bits >= 64U) {
                return SIZE_MAX; 
            }
            
            /*
             * Can handle up to /65 subnets (2^63 hosts) before too big
             */
            return (size_t)1U << host_bits;

        default:
            /*
             *  Unsupported or unknown family
             */
            return 0U;
    }
}

