/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <sys/socket.h>

/**
 * Check if a cidr belongs to the IPv4 address family.
 *
 * :param cidr: The cidr to validate.
 *
 * :returns: true if the cidr is IPv4
 */
bool ct_is_ipv4(const CtCidr *cidr) {
    if (!cidr) {
        return false;
    }
    return cidr->addr.family == AF_INET;
}

/**
 * Check if an cidr belongs to the IPv6 address family.
 *
 * :param cidr: The cidr to validate.
 *
 * :returns: true if the cidr is IPv6
 */
bool ct_is_ipv6(const CtCidr *cidr) {
    if (!cidr) {
        return false;
    }
    return cidr->addr.family == AF_INET6;
}

