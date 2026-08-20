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
     * Starting cidrs : 
     *   10.241.0.0/16 10.245.64.0/18 10.251.0.0/16 192.16.2.105/32 192.16.214.104/32
     * excluded
     *   10.241.0.0/15 10.244.0.0/15 10.250.0.0/15
     *
     * Result:
     *  192.16.2.105/32 192.16.214.104/32
     */
    CtCidrs cidrs = {};
    CtCidrs excld = {};
    const char *all_str = "10.241.0.0/16,10.245.64.0/18,10.251.0.0/16,192.16.2.105/32,192.16.214.104/32";
    const char *exl_str = "10.241.0.0/15,10.244.0.0/15,10.250.0.0/15";
    size_t count = 5U;
    size_t count_exl = 3U;
    size_t count_res = 2U;

    /*
     * Initialize
     */
    if (ct_flat_buffer_to_cidrs(all_str, count, &cidrs) != 0) {
        printf("[FAIL] maling CtCidrs\n");
        failed++;
        goto cleanup;
    }

    if (ct_flat_buffer_to_cidrs(exl_str, count_exl, &excld) != 0) {
        printf("[FAIL] maling CtCidrs\n");
        failed++;
        goto cleanup;
    }

    /*
     * Do the exclude
     */
    if (ct_exclude_cidrs(&cidrs, &excld) != 0) {
        printf("[FAIL] ct_exclude_cidrs had an error.\n");
        failed++;
        goto cleanup;
    }

    if (cidrs.count == count_res) {
        printf(" Good - result has %zu cidrs\n", count_res);
    } else {
        printf(" Bad - result has %zu cidrs but should be %zu\n", cidrs.count, count_res);
        failed ++;
    } 

    /*
     * Check result Expect: 192.16.2.105/32 192.16.214.104/32
     */
    for (size_t i = 0; i < cidrs.count; i++) {
        if (ct_cidr_to_str_r(&cidrs.blocks[i], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }
        printf("  -> %s\n", cidr_str); 
    }

    /*
     * Check result is correct.
     */
    if (cidrs.count == 2U) {       // NOLINT(readability-magic-numbers)
        if (ct_cidr_to_str_r(&cidrs.blocks[0], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }

        if (strcmp(cidr_str, "192.16.2.105/32") == 0) {
            printf("[SUCCESS] First output is correct\n");
        } else {
            printf("[FAIL] First cidr is wrong.\n");
            failed++;
        }

        if (ct_cidr_to_str_r(&cidrs.blocks[1], cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error from ct_cidr_to_str_r\n");
            failed++;
            goto cleanup;
        }

        if (strcmp(cidr_str, "192.16.214.104/32") == 0) {
            printf("[SUCCESS] Second output is correct\n");
        } else {
            printf("[FAIL] Second cidr is wrong.\n");
            failed++;
        }
    } 

cleanup:
    ct_free_cidrs(&cidrs);
    ct_free_cidrs(&excld);
    return (failed > 0) ? 1 : 0;
}

