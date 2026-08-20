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
    char cidr_str[INET6_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    printf("=== Testing ct_exclude_cidrs ===\n");

    /*
     * base cidr pool: 192.168.1.0/24
     */
    CtCidrs pool = {};
    CtCidrs excluded = {};

    if (!ct_allocate_cidrs(1, &pool)) {
        printf("[FAIL] Memory error\n");
        failed ++;
        goto cleanup;
    }

    if (!ct_allocate_cidrs(1, &excluded)) {
        printf("[FAIL] Memory error\n");
        ct_free_cidrs(&pool);
        failed ++;
        goto cleanup;
    }

    /*
     * pool has: ["192.168.1.0/24"]
     */
    if (ct_str_to_cidr_block("192.168.1.0/24", &pool.blocks[0]) != 0) {
        printf("[FAIL] Error from ct_str_to_cidr\n");
        failed++;
        goto cleanup;
    }

    /*
     * Exclude : 192.168.1.0/25 
     * - This removes the first half of the /24 network
     */
    if (ct_str_to_cidr_block("192.168.1.0/25", &excluded.blocks[0]) != 0) {     // NOLINT(readability-magic-numbers)
        printf("[FAIL] Error from ct_str_to_cidr\n");
        failed++;
        goto cleanup;
    }

    /*
     * Rewmove the excluded cidrs
     */
    if (ct_exclude_cidrs(&pool, &excluded) < 0) {
        printf("[FAIL] ct_exclude_cidrs had an error.\n");
        failed++;
        goto cleanup;
    }


    printf("[OK] ct_exclude_cidrs returned %zu new cidr blocks:\n", pool.count);
    for (size_t i = 0; i < pool.count; i++) {
        if (ct_cidr_to_str_r(&pool.blocks[i], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }
        /* 
         * Expect: 192.168.1.128/25
         */
        printf("  -> %s\n", cidr_str); 
    }

    /*
     * Check result is correct.
     */
    if (pool.count == 1 && pool.blocks[0].prefix == 25) {       // NOLINT(readability-magic-numbers)
        if (ct_cidr_to_str_r(&pool.blocks[0], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }

        if (strcmp(cidr_str, "192.168.1.128/25") == 0) {
            printf("[SUCCESS] exclude worked correctly!\n");
        } else {
            printf("[FAIL] Wrong cidr subnets returned.\n");
            failed++;
        }
    } else {
        printf("[FAIL] Wrong cidr count or prefix size.\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&pool);
    ct_free_cidrs(&excluded);
    return (failed > 0) ? 1 : 0;
}

