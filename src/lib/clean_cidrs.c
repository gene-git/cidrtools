/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdlib.h>
#include <sys/socket.h>

/**
 * Sanitizes a all the cidrs in in the list. 
 * Clamps illegal prefixes and zeroes out any host bits.
 *
 * Note:
 * - Unrecognized families are ignored.
 * - Empty lists are not an error, but cidrs being a nullptr is.
 *
 * :param cidrs: The array of cidrs to be "cleaned"
 *
 * :returns: 0 on success, or -1 on invalid parameters.
 */
int ct_clean_cidrs(CtCidrs *cidrs) {

    if (!cidrs) {
        return -1;
    }
    
    if (!cidrs->blocks || cidrs->count < 1) {
        return 0;
    }

    for (size_t i = 0; i < cidrs->count; i++) {
        CtCidr *curr = &cidrs->blocks[i];

        switch (curr->addr.family) {
            case AF_INET:
                if (curr->prefix > 32) {
                    curr->prefix = 32;
                }
                break;

            case  AF_INET6:
                if (curr->prefix > 128) {
                    curr->prefix = 128;
                }
                break;

            default:
                /*
                 * Skip over unrecognized families
                 */
                continue;
        }

        (void)ct_cidr_fix_host_bits(curr);
    }

    return 0;
}


/**
 * Sanitizes one cidr.
 * Clamps illegal prefixes and zeroes out any host bits.
 *
 * :param cidrs: The array of cidrs to be "cleaned"
 * :returns: 0 on success, or -1 on invalid parameters.
 */
int ct_clean_cidr(CtCidr *cidr) {

    if (!cidr) {
        return -1;
    }
    
    switch (cidr->addr.family) {
        case AF_INET:
            if (cidr->prefix > 32) {
                cidr->prefix = 32;
            }
            break;

        case  AF_INET6:
            if (cidr->prefix > 128) {
                cidr->prefix = 128;
            }
            break;

        default:
            /*
             * Ignore unrecognized families
             */
            break;
    }

    (void)ct_cidr_fix_host_bits(cidr);

    return 0;
}
