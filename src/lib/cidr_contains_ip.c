/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Determines if an ``IP`` address falls inside the ``IP`` range spanned by a cidr block.
 *
 * Convenience wrapper of ct_cidr_contains_cidr().
 *
 * :param ip: The ``IP`` to check.
 * :param cidr: The cidr block.
 *
 * :returns: true if the IP lies inside the subnet boundaries, false otherwise.
 */
bool ct_cidr_contains_ip(const CtCidr *cidr, const CtAddress *ip) {

    if (!cidr || !ip) {
        return false;
    }

    CtCidr target = {};

    switch (ip->family) {
        case AF_INET:
            target.prefix = 32U;
            break;

        case AF_INET6:
            target.prefix = 128U;
            break;

        default:
            return false;
    }
    memcpy(&target.addr, ip, sizeof(CtAddress));

    return ct_cidr_contains_cidr(cidr, &target);
}

