/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== Testing split_by_family ===\n");
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t buflen = sizeof(buf);

    CtCidrs all = {};
    CtCidrs cidrs_v4 = {};
    CtCidrs cidrs_v6 = {};

    /*
     * Set up list
     * - put them in sorted order to make resutl check simpler
     */
    char *v4_str[] = {"10.0.0.0/24", "10.1.0.0/24", "10.2.0.0/24"}; 
    char *v6_str[] = {"2001:db8::/64", "2002:d00d::/56"}; 

    size_t count_v4 = 3U;
    size_t count_v6 = 2U;
    size_t count = count_v4 + count_v6;

    if (!ct_allocate_cidrs(count, &all)) {
        printf("[FAIL] Error allocating memory\n");
        return 1;
    }

    for (size_t i = 0; i < count_v4; i++) {
        ct_str_to_cidr_block(v4_str[i], &all.blocks[i]);
    }

    for (size_t i = 0; i < count_v6; i++) {
        ct_str_to_cidr_block(v6_str[i], &all.blocks[count_v4 + i]);
    }

    if (ct_split_by_family(&all, &cidrs_v4, &cidrs_v6) != 0) {
        printf("[FAIL] Error from ct_split_by_family\n");
        return 1;
    }

    /*
     * Check result count
     */
    if (cidrs_v4.count != count_v4 || cidrs_v6.count != count_v6) {
        printf("[FAIL] - wrong number of ipv4/ipv6: (%zu, %zu) got (%zu, %zu)\n",
            count_v4, count_v6, cidrs_v4.count, cidrs_v6.count);
        return 1;
    }

    /*
     * Check result value
     * - should we compare string output or CtCidr == ... ?
     */
    for (size_t i = 0; i < count_v4; i++) {
        ct_cidr_to_str_r(&cidrs_v4.blocks[i], buf, buflen);

        if (strcmp(buf, v4_str[i]) != 0) {
            failed ++;
            printf("[FAIL] cidr_v4 mismatch: %s vs %s\n", buf, v4_str[i]);
        } else {
            printf("[OK] ipv4 match: %s\n", buf);
        }
    }

    for (size_t i = 0; i < count_v6; i++) {
        ct_cidr_to_str_r(&cidrs_v6.blocks[i], buf, buflen);

        if (strcmp(buf, v6_str[i]) != 0) {
            failed ++;
            printf("[FAIL] cidr_v4 mismatch: %s vs %s\n", buf, v6_str[i]);
        } else {
            printf("[OK] ipv6 match: %s\n", buf);
        }
    }

    ct_free_cidrs(&all);
    ct_free_cidrs(&cidrs_v4);
    ct_free_cidrs(&cidrs_v6);

    return (failed > 0) ? 1 : 0;
}


