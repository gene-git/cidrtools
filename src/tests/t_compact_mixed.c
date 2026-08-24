/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int test_mixed(void) {
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    printf("=== Testing ct_compact_mixed ===\n");

    /*
     * Special test compacting with 0/0
     * - compat to 0.0.0.0/0
     */
    CtCidrs cidrs = {};
    const char *flat_input = "10.0.0.0/24,10.0.1.0/24,fc00::/64,fc00:0:0:1::/64";
    size_t count = 4U;

    if (ct_flat_buffer_to_cidrs(flat_input, count, &cidrs) != 0) {
        printf("[FAIL] ct_flat_buffer_to_cidrs returned error\n");
        failed++;
        goto cleanup;
    }

    if (ct_compact(&cidrs) < 0) {
        printf("[FAIL] ct_compact returned error with 0/0\n");
        failed++;
        goto cleanup;
    }

    if (cidrs.count != 2U) {
        printf("[FAIL] ct_compact returned wrong count %zu instead of 2\n", cidrs.count);
        failed++;
        goto cleanup;
    }

    /*
     * ipv4
     */
    if (ct_cidr_to_str_r(&cidrs.blocks[0], buf, bufsz) != 0) {
        printf("[FAIL] Failed to extract cidr[0]\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(buf, "10.0.0.0/23") != 0) {
        printf("[FAIL] ct_compact got %s instead of 10.0.0.0/23\n", buf);
        failed++;
        goto cleanup;
    }

    /*
     * ipv6
     */
    if (ct_cidr_to_str_r(&cidrs.blocks[1], buf, bufsz) != 0) {
        printf("[FAIL] Failed to extract cidr[0]\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(buf, "fc00::/63") != 0) {
        printf("[FAIL] ct_compact got %s instead of fc00::/63\n", buf);
        failed++;
        goto cleanup;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}


int main(void) {
    int failed = 0;

    printf("=== Testing ct_compact to default 0/0 ===\n");

    failed = test_mixed();

    return (failed > 0) ? 1 : 0;
}

