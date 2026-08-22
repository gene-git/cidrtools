/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * In-place sort of a list of cidrs.
 *
 * :param cidrs: The cidr blocks to sort. Since this is done in place, the blocks are
 *               moved around within the CtCidrs structure.
 *
 * :returns: 0 on success, -1 otherwise.
 */
int ct_sort(CtCidrs *cidrs) {
    if (!cidrs) {
        return 0;
    }

    if (cidrs->count == 0 || !cidrs->blocks) {
        /*
         *  Nothing to sort
         */
        return 0;
    }

    /*
     * Sanity Check if memory footprint changed
     */
    uintptr_t diff = (uintptr_t)&(cidrs->blocks[1]) - (uintptr_t)&(cidrs->blocks[0]);
    if (diff != sizeof(CtCidr)) {
        printf("BUG: memory alignment mismatch! Step diff is %zu, but sizeof is %zu\n", diff, sizeof(CtCidr));
    }

    for(size_t i = 0; i < cidrs->count; i++) {
        char *str = ct_cidr_to_str(&(cidrs->blocks[i]));
        if (strncmp(str, "128.", 4) == 0) {
            printf("BUG FOUND: Item at index %zu is ALREADY 128 before sorting!\n", i);
        }
        free(str);
    }

    qsort(cidrs->blocks, cidrs->count, sizeof(CtCidr), ct_cidr_sort_compare);

    return 0;
}

