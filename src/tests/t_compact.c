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
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    printf("=== Testing ct_compact ===\n");
    /*
     * Setup an CtCdirs with overlapping/adjacent blocks
     * - 192.168.1.0/24
     * - 192.168.1.128/25 (subset of above - should vanish)
     * - 192.168.2.0/24   (adjacent to .1.0/24, but wait - can they merge? 
     *   Let's check: 192.168.1.0 and 192.168.2.0 cannot merge into a /23 because 
     *   they don't align on a /23 bit boundary. But 192.168.0.0/24 and 192.168.1.0/24 can!)
     */
    CtCidrs cidrs = {};
    if (!ct_allocate_cidrs(3, &cidrs)) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");

    }
    
    /*
     * cidr 0: 192.168.0.0/24
     */
    if (ct_str_to_cidr_block("192.168.0.0/24", &cidrs.blocks[0]) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    /* 
     * cidr 1: 192.168.1.0/24 (adjacent buddy to block 0 on a /23 boundary!)
     */
    if (ct_str_to_cidr_block("192.168.1.0/24", &cidrs.blocks[1]) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    /*
     * cidr 2: 192.168.0.32/27 (contained inside 192.168.0.0/24 - should vanish)
     */
    if (ct_str_to_cidr_block("192.168.0.32/27", &cidrs.blocks[2]) != 0) {
        printf("[FAIL] error from ct_str_to_cidr_block\n");
        failed++;
        goto cleanup;
    }

    if (ct_compact(&cidrs) < 0) {
        printf("[FAIL] ct_compact returned error\n");
        failed++;
        goto cleanup;
    }

    printf("[OK] Compacted cidr list down to %zu blocks:\n", cidrs.count);
    for (size_t i = 0; i < cidrs.count; i++) {
        if (ct_cidr_to_str_r(&cidrs.blocks[i], buf, bufsz) != 0) {
            printf("[FAIL] ct_cidr_to_str_r returned error\n");
        }
        printf("  -> %s\n", buf);
    }

    /*
     * Expected output: 192.168.0.0/23 (since /24 + /24 merged and took the /27 with it!)
     */
    if (cidrs.count == 1 && cidrs.blocks[0].prefix == 23) {      // NOLINT(readability-magic-numbers)
        printf("[SUCCESS] cidrs compacted correctly!\n");
    } else {
        printf("[FAIL] cidrs failed to compact correctly\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

