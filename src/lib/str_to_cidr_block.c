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

/**
 * Parses a text string into a CtCidr. Can be IPv4 or IPv6.
 * Checks and that the prefix >= prefix_min. Using prefix_min = 0U 
 * means all prefixes are permitted including 0.
 * 
 * For example "192.168.1.50/24". 
 *
 * Invalid IP addresses are set to "0.0.0.0/32" and -2 is returned.
 *
 * :param str: The string to be parsed.
 * :param cidr: The resultant CtCidr.
 * :param prefix_min: require the prefix to be >= this value
 * :returns: 0 on success, -2 if invalid cidr and -1 on error.
 */
int ct_str_to_cidr_block_limit(const char *str, CtCidr *cidr, size_t prefix_min) {
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
    if (parsed_prefix < prefix_min) {
        (void)fprintf(stderr, "** cidrtools ct_str_to_cidr_block : Reject prefix below minimum'%s'\n", str);
        return -1;
    }

    /*
     * Parse the IP string into an address.
     * - if bad IP then set to 0.0.0.0/32 and return -2
     */
    if (ct_str_to_ip_address(ip_buf, &cidr->addr) != 0) {
        cidr->prefix = 32U;
        cidr->addr.family = AF_INET;
        memset(&cidr->addr.addr, 0, sizeof(cidr->addr.addr));
        return -2;
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

/**
 * Parses a text string into a CtCidr. Can be IPv4 or IPv6.
 * 
 * Same as ct_str_to_cidr_block_limit(str, cidr, 0U)  
 * 
 * For example "192.168.1.50/24". 
 * Invalid IP addresses are set to "0.0.0.0/32" and -2 is returned.
 *
 * :param str: The string to be parsed.
 * :param cidr: The resultant CtCidr.
 * :returns: 0 on success, -2 if invalid cidr and -1 on an error.
 */
int ct_str_to_cidr_block(const char *str, CtCidr *cidr) {

    if (!str || !cidr) {
        return -1;
    }
    return ct_str_to_cidr_block_limit(str, cidr, 0U);
}

