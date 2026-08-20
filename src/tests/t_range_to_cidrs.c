/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Testing ct_range_to_cidrs ===\n");
    int failed = 0;

    /*
     * Test : 192.168.1.3 to 192.168.1.10
     *
     * Expected output blocks: 
     * - 192.168.1.3/32
     * - 192.168.1.4/30  (covers .4, .5, .6, .7)
     * - 192.168.1.8/31  (covers .8, .9,)
     * - 192.168.1.10/32
     *
     */
    CtAddress start_v4 = {};
    CtAddress end_v4 = {};
    const char *start_v4_str = "192.168.1.3";
    const char *end_v4_str = "192.168.1.10";

    if (ct_str_to_ip_address(start_v4_str, &start_v4) != 0) {
        printf("[FAIL] ct_str_to_ip_address returns error from %s\n", start_v4_str);
        return 1;
    }

    if (ct_str_to_ip_address(end_v4_str, &end_v4) != 0) {
        printf("[FAIL] ct_str_to_ip_address returns error from %s\n", end_v4_str);
        return 1;
    }

    CtCidrs cidrs = {};
    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    if (ct_range_to_cidrs(&start_v4, &end_v4, &cidrs) == 0) {
        printf("[OK] Got %zu blocks spanning the IPv4 range:\n", cidrs.count);

        for (size_t i = 0; i < cidrs.count; i++) {
            if (ct_cidr_to_str_r(&cidrs.blocks[i], cidr_str, cidr_str_size) != 0) {
                printf("[FAIL] ct_cidr_to_str_r error\n");
                goto cleanup;
            }

            printf("  -> Block %zu: %s\n", i, cidr_str);
        }
        if (cidrs.count != 4) {
            printf("[FAIL] Expected exactly 4 CIDR blocks but got %zu.\n", cidrs.count);
            failed++;
        }

    } else {
        printf("[FAIL] Range to cidrs returned error.\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

