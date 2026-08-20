/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <string.h>

/**
 * Checks an whether a cidr block is contained in any network in a list of cidrs.
 *
 * :param cidr: The input cidr to check.
 * :param cidrs: The list of cidrs to check agains.
 *
 * :returns: true if the target block is enclosed inside any block within the array list.
 */
bool ct_cidr_is_subnet(const CtCidr *cidr, const CtCidrs *cidrs) {

    if (!cidr || !cidrs || !cidrs->blocks || cidrs->count == 0) {
        return false;
    }

    for (size_t i = 0; i < cidrs->count; i++) {
        if (ct_cidr_contains_cidr(&cidrs->blocks[i], cidr)) {
            return true;
        }
    }

    /*
     * no match found
     */
    return false;
}

