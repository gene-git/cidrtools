/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Internal helper to isolate IPv4 host bits.
 */
static inline int ct_get_host_bits_v4(const CtCidr *cidr, CtAddress *addr) {
    uint32_t raw_ip = 0U;
    uint32_t net_mask = 0U;
    uint32_t host_mask = 0U;
    uint32_t host_bits = 0U;

    if (cidr->prefix > 32U) {
        return -1;
    }

    raw_ip = ntohl(cidr->addr.addr.v4.s_addr);
    
    /*
     * Network mask. Handle /0 case to avoid undefined bit shifts.
     */
    net_mask = (cidr->prefix == 0U) ? 0x00000000U : (0xFFFFFFFFU << (32U - cidr->prefix));
    
    /*
     * Invert mask to isolate the host bits 
     * e.g. /24 net_mask of 0xFFFFFF00 => host_mask 0x000000FF
     */
    host_mask = ~net_mask;
    host_bits = raw_ip & host_mask;

    addr->addr.v4.s_addr = htonl(host_bits);
    return 0;
}

/*
 * helper to isolate IPv6 host bits - 
 * Zeros out network bits and retains the host bits. Thats it.
 * It uses 32-bit blocks for max performance - same as we do in other parts
 * of the code.
 */
static inline int ct_get_host_bits_v6(const CtCidr *cidr, CtAddress *addr) {
    uint8_t full_blocks = 0U;
    uint8_t remaining_bits = 0U;
    uint8_t i = 0U;

    if (cidr->prefix > 128U) {
        return -1;
    }

    full_blocks = cidr->prefix / 32U;
    remaining_bits = cidr->prefix % 32U;

    /*
     * Full network blocks have no host bits (fill with 0)
     */
    for (i = 0; i < full_blocks; i++) {
        addr->addr.v6.s6_addr32[i] = 0x00000000U;
    }

    if (full_blocks < 4) {
        /*
         * Boundry block has both network and host bits.
         * - zero the network bits.
         */
        uint32_t host_chunk = ntohl(cidr->addr.addr.v6.s6_addr32[full_blocks]);
        uint32_t net_mask = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));
        uint32_t host_mask = ~net_mask;

        addr->addr.v6.s6_addr32[full_blocks] = htonl(host_chunk & host_mask);

        /*
         * Rest are all host bits - keep them
         */
        for (i = full_blocks + 1; i < 4; i++) {
            addr->addr.v6.s6_addr32[i] = cidr->addr.addr.v6.s6_addr32[i];
        }
    }

    return 0;
}

/**
 * Extracts the host bits of a cidr block. The bits past the prefix boundary.
 *
 * :param cidr: The cidr to examine.
 * :param addr: The output which has the host bits.
 *
 * :returns: 0 on success, or -1 on invalid parameters/prefixes.
 */
int ct_get_host_bits(const CtCidr *cidr, CtAddress *addr) {
    if (!cidr || !addr) {
        return -1;
    }

    addr->family = cidr->addr.family;

    switch (cidr->addr.family) {
        case AF_INET:
            return ct_get_host_bits_v4(cidr, addr);

        case AF_INET6:
            return ct_get_host_bits_v6(cidr, addr);

        default:
            /* 
             * Unknown family
             */
            return -1;
    }
}

/**
 * Extracts host bits of a cidr block and returns a formatted string.
 *
 * Returns allocated string, caller responsible for free()'ing the mem.
 *
 * Uses ct_get_host_bits() and ct_ip_address_to_str().
 *
 * :param cidr: The cidr to examine.
 *
 * :returns: String containing the formatted host bits.
 */
char *ct_format_host_bits(const CtCidr *cidr) {
    CtAddress host_addr = {};

    if (!cidr) {
        return nullptr;
    }

    memset(&host_addr, 0, sizeof(host_addr));
    
    if (ct_get_host_bits(cidr, &host_addr) != 0) {
        return nullptr;
    }

    return ct_ip_address_to_str(&host_addr);
}

