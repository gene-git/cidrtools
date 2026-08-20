/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

enum {
    PREFIX_96 = 96,
    NUM_IPS_IN_96 = 4294967296U,
};

int main(void) {
    printf("=== Testing num_ips() ===\n");
    int failed = 0;

    /*
     * Test 1: IPv4 
     */
    CtCidr block_v4 = { .prefix = 24 };
    block_v4.addr.family = AF_INET;

    if (ct_num_ips(&block_v4) == 256) {
        printf("[OK] Correct - 256 for IPv4 /24\n");
    } else {
        printf("[FAIL] Failed IPv4 /24 host count\n");
        failed++;
    }

    /*
     * Test 2: IPv6
     */
    size_t num_ips = NUM_IPS_IN_96;
    CtCidr block_v6_huge = { .prefix = PREFIX_96 };
    block_v6_huge.addr.family = AF_INET6;

    if (ct_num_ips(&block_v6_huge) == num_ips) {
        printf("[OK] Correct host count IPv6 /96 block\n");
    } else {
        printf("[FAIL] Failed get host count for /96 block\n");
        failed++;
    }

    /*
     * Test 3: IPv6 (overflows when count >= 2^64
     */
    block_v6_huge.prefix = 48;
    block_v6_huge.addr.family = AF_INET6;

    if (ct_num_ips(&block_v6_huge) == SIZE_MAX) {
        printf("[OK] Flagged overflow for wide IPv6 /48 block\n");
    } else {
        printf("[FAIL] Failed to catch overflow\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

