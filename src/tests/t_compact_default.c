/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int test_ipv4(void) {
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    printf("=== Testing ct_compact_compact to [0/0, ::/0] ===\n");

    /*
     * Special test compacting with 0/0
     * - compat to 0.0.0.0/0
     */
    CtCidrs cidrs = {};
    const char *flat_input = "10.0.0.0/24,192.168.1.0/24,10.1.2.3.0/22,0.0.0.0/0,2000::/64,::/0";
    size_t count_v4 = 4U;
    size_t count_v6 = 2U;
    size_t count = count_v4 + count_v6;

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
        printf("[FAIL] ct_compact returned error with 0/0\n");
        failed++;
        goto cleanup;
    }
    if (ct_cidr_to_str_r(&cidrs.blocks[0], buf, bufsz) != 0) {
        printf("[FAIL] Failed to extract cidr of 0/0\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(buf, "0.0.0.0/0") != 0) {
        printf("[FAIL] ct_compact got %s instead of 0.0.0.0/0\n", buf);
        failed++;
        goto cleanup;
    }
    if (ct_cidr_to_str_r(&cidrs.blocks[1], buf, bufsz) != 0) {
        printf("[FAIL] Failed to extract cidr of ::/0\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(buf, "::/0") != 0) {
        printf("[FAIL] ct_compact got %s instead of ::/0\n", buf);
        failed++;
        goto cleanup;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

static int test_ipv6(void) {
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    printf("=== Testing ct_compact to default 0/0 ===\n");

    /*
     * Special test compacting with 0/0
     * - compat to 0.0.0.0/0
     */
    CtCidrs cidrs = {};
    const char *flat_input = "fc00:77:77::1/128,food::doob::/64,::/0";
    size_t count = 3U;

    if (ct_flat_buffer_to_cidrs(flat_input, count, &cidrs) != 0) {
        printf("[FAIL] ct_flat_buffer_to_cidrs returned error\n");
        failed++;
        goto cleanup;
    }

    if (ct_compact(&cidrs) < 0) {
        printf("[FAIL] ct_compact returned error with ::/0\n");
        failed++;
        goto cleanup;
    }
    if (cidrs.count != 1U) {
        printf("[FAIL] ct_compact returned error with ::/0\n");
        failed++;
        goto cleanup;
    }
    if (ct_cidr_to_str_r(&cidrs.blocks[0], buf, bufsz) != 0) {
        printf("[FAIL] Failed to extract cidr of ::/0\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(buf, "::/0") != 0) {
        printf("[FAIL] ct_compact got %s instead of ::/0\n", buf);
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

    failed = test_ipv4();
    failed += test_ipv6();

    return (failed > 0) ? 1 : 0;
}

