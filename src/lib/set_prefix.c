/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <sys/socket.h>

/**
 * Update the prefix of a cidr block. This updates in place and zeroes out any host bits past the prefix length.
 *
 * The new prefix must satisfy the IP family limits. An IPv4 cannot be changed into an IPv6 by
 * setting a prefix longer than 32 which will lead to an error.
 *
 * :param cidr: The cidr whose prefix is to be modified.
 * :param prefix: The new prefix
 *
 * :returns: 0 on success, or -1 otherwise. 
 *           Errors can be from bad invalid input or the prefix exceeds protocol limits.
 */
int ct_cidr_set_prefix(CtCidr *cidr, uint8_t prefix) {

    if (!cidr) {
        return -1;
    }

    switch (cidr->addr.family) {

        case AF_INET:
            if (prefix > 32) {
                return -1;
            }
            break;

        case AF_INET6:
            if (prefix > 128) {
                return -1;
            }
            break;
    
        default:
            return -1;
    }

    cidr->prefix = prefix;

    /*
     * Sanitize to be sure host bits zerod out
     */
    return ct_cidr_fix_host_bits(cidr);
}

