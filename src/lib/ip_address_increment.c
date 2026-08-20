/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
//#include <arpa/inet.h>

/**
 * Find an IP address that is an incremental number beyond an IP address.
 *
 * :param addr: The starting address.
 * :param num: The number of IP address to add to the starting address.
 * :param addr_inc: The resultant IP address *num* IPs after the provided first IP address.
 *
 * :returns: 0 on success, or -1 on error/overflow boundaries.
 */
int ct_ip_address_increment(const CtAddress *addr, size_t num, CtAddress *addr_inc) {
    uint32_t host_ip = 0U;
    uint32_t new_ip = 0U;
    uint64_t carry = 0U;
    uint32_t host_val = 0U;

    if (!addr || !addr_inc) {
        return -1;
    }

    addr_inc->family = addr->family;

    switch (addr->family) {
        case AF_INET:
            host_ip = ntohl(addr->addr.v4.s_addr);
            
            /*
             * Check overflow of 32-bit
             */
            if ((uint64_t)host_ip + num > UINT32_MAX) {
                return -1; 
            }

            new_ip = host_ip + (uint32_t)num;
            addr_inc->addr.v4.s_addr = htonl(new_ip);
            return 0;

        case AF_INET6:
            /*
             * Use 32-bit blocks (max 4 loops instead of 16)
             */
            carry = (uint64_t)num;
            
            for (int i = 3; i >= 0; i--) {
                host_val = ntohl(addr->addr.v6.s6_addr32[i]);
                carry += host_val;
                addr_inc->addr.v6.s6_addr32[i] = htonl((uint32_t)(carry & 0xFFFFFFFFU));

                /*
                 * Get remaining overflow carry bits
                 */
                carry >>= 32U;
                
                /*
                 * if carry goes to zero, trailing network blocks stay untouched
                 */
                if (carry == 0ULL && addr_inc != addr) {
                    /*
                     * Copy remaining untouched network blocks
                     */
                    for (int j = i - 1; j >= 0; j--) {
                        addr_inc->addr.v6.s6_addr32[j] = addr->addr.v6.s6_addr32[j];
                    }
                    return 0;
                }
            }

            /*
             * If carry remains after cascading left, then address overflowed past ffff:...:ffff
             */
            if (carry > 0ULL) {
                return -1;
            }
            return 0;

        default:
            /* 
             * Unsupported or uninitialized family 
             */
            return -1;
    }
}


