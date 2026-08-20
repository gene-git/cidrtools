/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdio.h>

int main(void) {
    printf("=== Testing cidr_contains_ip ===\n");
    int failed = 0;

    /*
     * Test 1: IPv4
     */
    CtCidr v4_cidr = {};
    CtCidr match_v4 = {};
    CtCidr miss_v4 = {};

    if (ct_str_to_cidr_block("192.168.10.0/24", &v4_cidr) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        return 1;
    }

    if (ct_str_to_cidr_block("192.168.10.55/32", &match_v4) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        return 1;
    }

    if (ct_str_to_cidr_block("192.168.11.55/32", &miss_v4) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        return 1;
    }

    if (ct_cidr_contains_cidr(&v4_cidr, &match_v4)) {
        printf("[OK] Correctly matched 192.168.10.55 inside the cidr block\n");
    } else {
        printf("[FAIL] Missed a valid IPv4 network child node\n");
        failed++;
    }

    if (!ct_cidr_contains_cidr(&v4_cidr, &miss_v4)) {
        printf("[OK] Correctly rejected 192.168.11.55 not inside the cidr block\n");
    } else {
        printf("[FAIL] Wrong - this is not inside the idr block\n");
        failed++;
    }

    /*
     * Test 2: IPv6
     */
    CtCidr v6_cidr = {};
    CtCidr match_v6 = {};
   
    if (ct_str_to_cidr_block("2001:db8:ffff::/64", &v6_cidr) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        return 1;
    }

    if (ct_str_to_cidr_block("2001:db8:ffff::abcd/128", &match_v6) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        return 1;
    }

    if (ct_cidr_contains_cidr(&v6_cidr, &match_v6)) {
        printf("[OK] Correctly matched 2001:db8:ffff::abcd inside /64 block\n");
    } else {
        printf("[FAIL] Missed a valid IPv6 subnet\n");
        failed++;
    }

    /*
     * Test 3: Cross Family check
     */
    if (!ct_cidr_contains_cidr(&v6_cidr, &match_v4)) {
        printf("[OK] Correctly rejected ipv4 block not within the ipv6 block)\n");
    } else {
        printf("[FAIL] Cross family check failed\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

