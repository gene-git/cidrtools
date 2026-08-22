/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

enum { T_PREFIX_28 = 28U, };

int main(void) {
    int failed = 0;
    char cidr_str[INET6_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);
    CtCidrs cidrs_1 = {};
    CtCidrs cidrs_2 = {};
    CtCidrs cidrs = {};

    printf("=== Testing ct_cidrs_intersection ===\n");

    /*
     * Parent 0: 192.168.1.0/24
     * Parent 1: 2001:db8:abcd::/48
     */
    const char *cidr_1_strings[] = {"192.168.1.0/24", "10.1.0.0/24"};
    const char *cidr_2_strings[] = {"10.1.0.0/22", "100.100.100.0/23"};
    const char *cidr_inters[] = {"10.1.0.0/24"};

    size_t cidr_1_len = sizeof(cidr_1_strings) / sizeof(cidr_1_strings[0]);
    size_t cidr_2_len = sizeof(cidr_2_strings) / sizeof(cidr_2_strings[0]);
    size_t cidr_inters_len = sizeof(cidr_inters) / sizeof(cidr_inters[0]);

    if (ct_str_array_to_cidrs(cidr_1_strings, cidr_1_len, &cidrs_1) != 0) {
        failed++;
        goto cleanup;
    }

    if (ct_str_array_to_cidrs(cidr_2_strings, cidr_2_len, &cidrs_2) != 0) {
        failed++;
        goto cleanup;
    }

    if (ct_cidrs_intersection(&cidrs_1, &cidrs_2, &cidrs) != 0) {
        failed++;
        goto cleanup;
    }

    /*
     * Print resulting cids
     */
    printf(" Result of interection has %zu elems\n", cidrs.count);
    for (size_t i = 0; i < cidrs.count; i++) {
        if (ct_cidr_to_str_r(&cidrs.blocks[i], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }
        printf(" %s\n", cidr_str);
    } 

    /*
     * Check result;
     */
    if (cidrs.count != cidr_inters_len) {
        printf("[FAIL] intersection has %zu elems, expected %zu\n", cidrs.count, cidr_inters_len);
        failed++;
        goto cleanup;
    }

    for (size_t i = 0; i < cidr_inters_len; i++) {
        if (ct_cidr_to_str_r(&cidrs.blocks[i], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }

        if (strcmp(cidr_str, cidr_inters[i]) != 0) {
            printf("[FAIL] wrong intersection %s should be %s\n", cidr_str, cidr_inters[i]);
            failed++;
        }
    }

    if (failed == 0) {
        printf("[OK] All correct\n");
    }

cleanup:
    ct_free_cidrs(&cidrs_1);
    ct_free_cidrs(&cidrs_2);
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

