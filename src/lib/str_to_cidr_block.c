/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>

/**
 * Parses a text string into a CtCidr. Can be IPv4 or IPv6.
 * 
 * For example "192.168.1.50/24". 
 *
 * :param str: The string to be parsed.
 * :param cidr: The resultant CtCidr.
 *
 * :returns: 0 on success, or -1 if the string is invalid or not a valid cidr.
 */
int ct_str_to_cidr_block(const char *str, CtCidr *cidr) {
    char ip_buf[INET6_ADDRSTRLEN] = {};
    uint8_t parsed_prefix = 0U;

    if (!str || !cidr) {
        return -1;
    }

    /*
     * Split into IP strin and a prefix number
     */
    if (ct_str_to_cidr_parts(str, ip_buf, sizeof(ip_buf), &parsed_prefix) != 0) {
        return -1;
    }

    /*
     * safety guard - disallow prefix <= 4
     * - add debug log here.
     */
    if (parsed_prefix <= 4U) {
        (void)fprintf(stderr, "** cidrtools ct_str_to_cidr_block : Rejected truncated prefix string '%s'\n", str);
        return -1;
    }

    /*
     * Parse the IP string into an address.
     */
    if (ct_str_to_ip_address(ip_buf, &cidr->addr) != 0) {
        return -1;
    }

    /*
     * Ensure a "sensible" prefix
     * - we do leave default route prefix of 0 alone. Perhaps that is bad idea?
     */
    switch (cidr->addr.family) {
        case AF_INET:
            if (parsed_prefix > 32U) {
                parsed_prefix = 32U;
            }
            break;

        case AF_INET6:
            if (parsed_prefix > 128U) {
                parsed_prefix = 128U;
            }
            break;

        default:
            break;
    }

    cidr->prefix = parsed_prefix;
    return ct_cidr_fix_host_bits(cidr);
}

