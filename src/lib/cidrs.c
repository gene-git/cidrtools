/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdlib.h>
#include <string.h>


/**
 * Frees memory allocations inside a CtCidrs struct.
 * 
 * Resets internal pointer to nullptr and count to 0.
 *
 * :param cidrs: Pointer to the CtCidrs
 *
 * :returns: nothing - it's a void function
 */
void ct_free_cidrs(CtCidrs *cidrs) {
    if (!cidrs) {
        return;
    }

    if (cidrs->blocks) {
        free(cidrs->blocks);
        cidrs->blocks = nullptr;
    }
    cidrs->count = 0U;
}

/**
 * Add a cidr block to the list of cidrs.
 *
 * :param cidrs: Add the new cidr to this list of cidrs.
 * :param cidr: The cidr to add
 *
 * :returns: true on success, false on error
 */
bool ct_add_cidr_to_cidrs(CtCidrs *cidrs, const CtCidr *cidr) {
    size_t new_count = cidrs->count + 1U;

    void *tmp = realloc(cidrs->blocks, new_count * sizeof(CtCidr));
    if (!tmp) {
        return false;
    }
    cidrs->blocks = (CtCidr *)tmp;
    cidrs->blocks[cidrs->count].addr = cidr->addr;
    cidrs->blocks[cidrs->count].prefix = cidr->prefix;
    cidrs->count = new_count;
    return true;
}

/**
 * Allocate 'count' cidr blocks cidrs.
 *
 * Note new memory is set to zero
 *
 * :param count: Allocate ``count`` blocks
 * :param cidrs: The cidr list to modify.
 *
 * :returns: true on success, false on error
 */
bool ct_allocate_cidrs(size_t count, CtCidrs *cidrs) {
    void *ptr = nullptr;

    /*
     * Free
     */
    if (count == 0U) {
        if (cidrs->count > 0) {
            if (cidrs->blocks) {
                free(cidrs->blocks);
                cidrs->blocks = nullptr;
            }
            cidrs->count = 0U;
        }
        return true;
    }

    /*
     * New
     */
    if (cidrs->count == 0 || !cidrs->blocks) {
        ptr = calloc(count, sizeof(CtCidr));
        if (!ptr) {
            return false;
        }
        cidrs->blocks = (CtCidr *)ptr;
        cidrs->count = count;
        return true;
    }

    /*
     * Update 
     */
    size_t count_orig = cidrs->count;
    ptr = realloc(cidrs->blocks, count * sizeof(CtCidr));
    if (!ptr) {
        return false;
    }
    cidrs->blocks = (CtCidr *)ptr;
    cidrs->count = count;

    /*
     * If increased, zero out the new part
     */
    if (count > count_orig) {
        ptr = cidrs->blocks + count_orig;
        memset(ptr, 0, (count - count_orig) * sizeof(CtCidr));
    }
    return true;
}

