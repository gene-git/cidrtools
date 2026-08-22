/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdio.h>

enum { T_PREFIX_28 = 28U, };

int main(void) {
    int failed = 0;
    CtCidrs cidrs = {};

    printf("=== Testing ct_cidr_is_subnet ===\n");

    if (!ct_allocate_cidrs(2, &cidrs)) {
        printf("[FAIL] Error alloc memory\n");
        return 1;
    }

    /*
     * Parent 0: 192.168.1.0/24
     * Parent 1: 2001:db8:abcd::/48
     */
    const char *cidr_0 = "192.168.1.0/24";
    const char *cidr_1 = "2001:db8:abcd::/48";

    if (ct_str_to_cidr_block(cidr_0, &cidrs.blocks[0]) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (ct_str_to_cidr_block(cidr_1, &cidrs.blocks[1]) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    /*
     * Test 1: Match IPv4 Subnet
     */
    CtCidr test_v4_sub = {};
    const char *test_v4_sub_str = "192.168.1.32/28";

    if (ct_str_to_cidr_block(test_v4_sub_str, &test_v4_sub) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (ct_cidr_is_subnet(&test_v4_sub, &cidrs)) {
        printf("[OK] Correctly matched %s inside list of cidrs\n", test_v4_sub_str);

    } else {
        printf("[FAIL] Failed to match embedded IPv4 subnet\n");
        failed++;
    }

    /*
     *  Test 2: Match IPv6
     */
    CtCidr test_v6_sub = {};
    const char *test_v6_sub_str = "2001:db8:abcd:1234::/64";

    if (ct_str_to_cidr_block(test_v6_sub_str, &test_v6_sub) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (ct_cidr_is_subnet(&test_v6_sub, &cidrs)) {
        printf("[OK] Correctly matched %s inside /48 block\n", test_v6_sub_str);
    } else {
        printf("[FAIL] Failed to match IPv6 subnet\n");
        failed++;
    }

    /*
     * Test 3: IPv4 -  Outside of Subnet
     */
    CtCidr test_v4_out = {};
    const char *test_v4_out_str = "192.168.2.0/24";

    if (ct_str_to_cidr_block(test_v4_out_str, &test_v4_out) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (!ct_cidr_is_subnet(&test_v4_out, &cidrs)) {
        printf("[OK] Correctly rejected unrelated %s subnet\n", test_v4_out_str);

    } else {
        printf("[FAIL] Incorrectly matched out-of-bounds cidr\n");
        failed++;
    }

    /*
     * Test 4: IPv6 - outside of subnet
     */
    CtCidr test_v4_wide = {};
    const char *test_v4_wide_str = "192.168.1.0/16";

    if (ct_str_to_cidr_block(test_v4_wide_str, &test_v4_wide) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (!ct_cidr_is_subnet(&test_v4_wide, &cidrs)) {
        printf("[OK] Correctly rejected a /16 segment is not inside a /24 parent\n");
    } else {
        printf("[FAIL] Wrong claiming a /16 is inside a /24\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

