/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct split_check {
    CtCidr *cidr;
    uint8_t prefix;
    size_t count;
    char *first;
    char *last;
};

static int split_cidr_by_prefix(struct split_check *split) {
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);
    CtCidrs *cidrs = nullptr;
    CtCidr *cidr = split->cidr;
    uint8_t prefix = split ->prefix;
    size_t count = split ->count;

    cidrs = ct_subnets_split(cidr, prefix);
    if (!cidrs) {
        printf("[FAIL] ct_subnets_split returned a error for prefix  %du\n", prefix);
        return 1;
    }

    printf("[OK] Success. Split cidr into %zu subnets with prefix %du\n", cidrs->count, prefix);

    for (size_t i = 0; i < cidrs->count; i++) {
        if (ct_cidr_to_str_r(&cidrs->blocks[i], buf, bufsz) != 0) {
            printf("[FAIL] ct_cidr_to_str_r error\n");
            failed ++;
            goto cleanup;
        }
        printf("  -> Subnet %zu: %s\n", i, buf);
    }

    if (cidrs->count != count) {
        printf("[FAIL] Expected %zu split subnets not %zu\n", count, cidrs->count);
        failed++;
        goto cleanup;
    }

    /*
     * Check first and last element
     */
    if (ct_cidr_to_str_r(&cidrs->blocks[0], buf, bufsz) != 0) {
        printf("[FAIL] ct_cidr_to_str_r error\n");
        failed ++;
        goto cleanup;
    }

    if (strcmp(buf, split->first) == 0) { 
        printf("[SUCCESS] IPv4 subnet split match first\n");
    } else {
        printf("[FAIL] IPv4 subnet split failed first.\n");
        failed++;
    }

    /*
     * Check last element
     */
    if (ct_cidr_to_str_r(&cidrs->blocks[count - 1], buf, bufsz) != 0) {
        printf("[FAIL] ct_cidr_to_str_r error\n");
        failed ++;
        goto cleanup;
    }

    if (strcmp(buf, split->last) == 0) {
        printf("[SUCCESS] IPv4 subnet split match last\n");
    } else {
        printf("[FAIL] IPv4 subnet split failed last.\n");
        failed++;
    }

cleanup:
    if (cidrs) {
        ct_free_cidrs(cidrs);
        free((void *)cidrs);
    }

    return failed;
}

int main(void) {
    int failed = 0;
    char *cidr_str = nullptr;
    CtCidrs *cidrs_invalid = nullptr;
    struct split_check split= {};

    printf("=== Testing ct_subnets_split ===\n");

    /*
     * Test 1: Split 192.168.1.0/24 into /26 subnets 
     * - Should get 4 blocks
     */
    CtCidr cidr_24 = {};
    cidr_str = "192.168.1.0/24";
    if (ct_str_to_cidr_block(cidr_str, &cidr_24) != 0) {
        printf("[FAIL] ct_str_to_cidr_block failed on %s\n", cidr_str);
    }

    split.cidr = &cidr_24;
    split.prefix = 26U;                       // NOLINT(readability-magic-numbers)
    split.count = 4U;                         // NOLINT(readability-magic-numbers)
    split.first = "192.168.1.0/26";
    split.last = "192.168.1.192/26";

    failed += split_cidr_by_prefix(&split);

    /*
     * Test 2: Invalid request
     * - Try to split a /24 into a /16 is an error.
     */
    cidrs_invalid = ct_subnets_split(&cidr_24, 16);
    if (!cidrs_invalid || cidrs_invalid->count == 0) {
        printf("[OK] Success - correctly rejected invalid prefix split.\n");
    } else {
        printf("[FAIL] Failed to reject an invalid prefix split request.\n");
        failed++;
    }

    if (cidrs_invalid) {
        ct_free_cidrs(cidrs_invalid);
        free((void *)cidrs_invalid);
    }

    return (failed > 0) ? 1 : 0;
}

