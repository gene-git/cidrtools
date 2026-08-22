/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <string.h>

/**
 * Return the intersection of two sets of CtCirs or nullptr if none.
 *
 * Note that:
 * - Both cidrs1 and cidrs2 are sorted in place.
 * - The result is not compacted, caller can compact if desited.
 *
 * :param cidrs1: The first set of cidrs
 * :param cidrs2: The second set of cidrs
 * :param cidrs: The intersection set of cidrs are added to this set.
 * :returns: 0 if no errors otherwise -1 (memory allocation error)
 */
int ct_cidrs_intersection(CtCidrs *cidrs1, CtCidrs *cidrs2, CtCidrs *cidrs) {

    if (!cidrs1 || !cidrs2 || !cidrs1->blocks || !cidrs2->blocks) {
        return 0;
    }

    if (cidrs1->count == 0 || cidrs2->count == 0) {
        return 0;
    }

    /*
     * Intersecting cidrs:
     * - both cidrs are the same
     * - smaller of the 2 is a subnet of the larget
     */
    size_t i = 0;
    size_t j = 0;

    /*
     * Sort (in place)
     */
    if (ct_sort(cidrs1) != 0) {
        return -1;
    }
    if (ct_sort(cidrs2) != 0) {
        return -1;
    }

    while (i < cidrs1->count && j < cidrs2->count) {
        CtCidr *cidr_1 = &cidrs1->blocks[i];
        CtCidr *cidr_2 = &cidrs2->blocks[j];

        /*
         * nothing to do if different IP families
         */
        if (cidr_1->addr.family != cidr_2->addr.family) {
            if (cidr_1->addr.family < cidr_2->addr.family) {
                i++;
            } else {
                j++;
            }
        }

        CtCidr *cidr_intersects = nullptr;

        int comp = ct_cidr_sort_compare((void *)cidr_1, (void *)cidr_2);
        if (comp == 0) {
            cidr_intersects = cidr_1;
            i++;
            j++;

        } else if (ct_cidr_contains_cidr(cidr_1, cidr_2)) {
            cidr_intersects = cidr_2;
            j++;

        } else if (ct_cidr_contains_cidr(cidr_2, cidr_1)) {
            cidr_intersects = cidr_1;
            i++;
        } else {
            i++;
            j++;
        }

        if (cidr_intersects) {
            if (!ct_add_cidr_to_cidrs(cidrs, cidr_intersects)) {
                return -1;
            }
        }
    }

    return 0;
}

