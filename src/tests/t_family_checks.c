/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdio.h>
#include <sys/socket.h>

int main(void) {
    printf("=== Testing ct_is_ipv4/ct_is_ipv6 ===\n");
    int failed = 0;

    CtCidr test_v4 = { .addr = { .family = AF_INET } };
    CtCidr test_v6 = { .addr = { .family = AF_INET6 } };

    /*
     * Test 1: Confirm IPv4 valid and invalid both correct.
     */
    if (ct_is_ipv4(&test_v4) && !ct_is_ipv6(&test_v4)) {
        printf("[OK] IPv4 validation correct\n");
    } else {
        printf("[FAIL] IPv4 validation incorrect\n");
        failed++;
    }

    /*
     * Test 2: Confirm IPv6 valid and invalid both correct.
     */
    if (ct_is_ipv6(&test_v6) && !ct_is_ipv4(&test_v6)) {
        printf("[OK] IPv6 validation correct\n");
    } else {
        printf("[FAIL] IPv6 validation incorrect\n\n");
        failed++;
    }

    /*
     * Test 3: Null Check
     */
    if (!ct_is_ipv4(nullptr) && !ct_is_ipv6(nullptr)) {
        printf("[OK] nullptr cidrs check passes\n");
    } else {
        printf("[FAIL] nullptr cidr check failed\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

