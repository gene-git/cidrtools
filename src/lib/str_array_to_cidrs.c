/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stddef.h>

/**
 * Parses a list of string into a CtCidrs. Can be IPv4 or IPv6.
 *
 * :param str_array: The list of cidr strings to be parsed.
 * :param count: The number of cidrs in the array
 * :param cidrs: The resultant CtCidrs.
 *
 * :returns: 0 on success, otherwise -1
 */
int ct_str_array_to_cidrs(const char **str_array, size_t count, CtCidrs *cidrs) {
    if (!str_array || !cidrs) {
        return -1;
    }
    
    /* 
     * Allocate memory
     */
    if (!ct_allocate_cidrs(count, cidrs)) {
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        if (str_array[i] == nullptr) {
            continue;
        }
        
        /* 
         * Parse into the allocated block
         */
        int rc = ct_str_to_cidr_block(str_array[i], &cidrs->blocks[i]);
        if (rc != 0) {
            /*
             * Cleanup on failure to avoid memory leaks
             */
            ct_free_cidrs(cidrs);
            return rc;
        }
    }
    return 0;
}

/**
 * Create a list of cidr strings from CtCidrs
 *
 * Use ct_cidrs_to_flat_buffer() - it is much faster.
 *
 * :param cidrs: The CtCidrs struct with count 
 * :param dest_array: Where to save the strings
 */
int ct_cidrs_to_str_array(const CtCidrs *cidrs, char **dest_array) {
    if (!cidrs || !dest_array) {
        return -1;
    }

    for (size_t i = 0; i < cidrs->count; i++) {
        dest_array[i] = ct_cidr_to_str(&cidrs->blocks[i]);
        if (dest_array[i] == nullptr) {
            return -1; 
        }
    }
    return 0;
}

