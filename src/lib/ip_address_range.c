/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <sys/socket.h>


/**
 * Computes the first and last IP addresses of a cidr block given an IP address and a prefix.
 *
 * Same functionality as ct_cidr_to_range() which this uses.
 * 
 * :param addr: The IP address to consider
 * :param prefix: The prefix of the subnet.
 * :param first: The first IP address in the range.
 * :param last: The last IP address in the range.
 *
 * :returns: 0 on success, or -1 on invalid input.
 */
int ct_ip_address_range(const CtAddress *addr, uint8_t prefix, CtAddress *first, CtAddress *last) {

    if (!addr || !first || !last) {
        return -1;
    }

    CtCidr cidr = {};
    switch (addr->family) {
        case AF_INET:
            cidr.prefix = 32U;
            break;

        case AF_INET6:
            cidr.prefix = 128U;
            break;

        default:
            return -1;
    }
    cidr.prefix = prefix;
    return ct_cidr_to_range(&cidr, first, last);

}

