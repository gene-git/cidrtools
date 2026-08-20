/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * Dev Note: 
 * See cidr_to_range.c for notes about bit operations, masking
 * and using 32 bit blocks versus 8 bit blocks.
 */

/**
 * clear IPv4 host bits
 */
static inline int ct_cidr_fix_host_bits_v4(CtCidr *cidr) {
    uint32_t host_ip = 0U;
    uint32_t mask = 0U;
    uint32_t clean_ip = 0U;

    if (cidr->prefix > 32U) {
        return -1;
    }

    host_ip = ntohl(cidr->addr.addr.v4.s_addr);
    
    /*
     *  bitmask for values 0 through 32
     */
    mask = (cidr->prefix == 0U) ? 0x00000000U : (0xFFFFFFFFU << (32U - cidr->prefix));
    clean_ip = host_ip & mask;

    cidr->addr.addr.v4.s_addr = htonl(clean_ip);
    return 0;
}

/**
 * clear IPv6 host bits using (high-performance) 32-bit blocks.
 * Reduces a 16-iteration loop down to 4 native register operations.
 */
static inline int ct_cidr_fix_host_bits_v6(CtCidr *cidr) {
    uint8_t full_blocks = 0;
    uint8_t remaining_bits = 0;
    uint8_t i = 0;

    if (cidr->prefix > 128U) {
        return -1;
    }

    full_blocks = cidr->prefix / 32U;
    remaining_bits = cidr->prefix % 32U;

    /*
     * Fully network blocks require no modifications - only zero the host bit.
     * May be one block which has some network bits and some host bits.
     * After that they are all host bits.
     */
    if (full_blocks < 4) {
        /*
         * "border" block has both network and host bits.
         * - Zero out the host bits
         */
        uint32_t host_block = ntohl(cidr->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t mask = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));
        
        cidr->addr.addr.v6.s6_addr32[full_blocks] = htonl(host_block & mask);

        /*
         * Remaining are all host bits: clear them all to 0x00000000
         */
        for (i = full_blocks + 1; i < 4; i++) {
            cidr->addr.addr.v6.s6_addr32[i] = 0x00000000U;
        }
    }

    return 0;
}

/**
 * Modifies the cidr in-place to clear any host bits past the prefix boundary.
 *
 * :param cidr: The cidr to fix
 *
 * :returns: 0 on success, or -1 on error such as invalid input or a bad prefix.
 */
int ct_cidr_fix_host_bits(CtCidr *cidr) {
    if (!cidr) {
        return -1;
    }

    switch (cidr->addr.family) {
        case AF_INET:
            return ct_cidr_fix_host_bits_v4(cidr);

        case AF_INET6:
            return ct_cidr_fix_host_bits_v6(cidr);

        default:
            /* Unknown family or uninitialized memory */
            return -1;
    }
}

