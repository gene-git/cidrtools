/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


/*
 * Helper to check cidr matches
 */
static bool check_cidr(CtCidr *cidr, int family, size_t prefix, const char *cidr_str) {

    if (cidr->addr.family != family) {
        return false;
    }

    if (cidr->prefix != prefix) {
        return false;
    }

    char cidr_str_x[INET6_ADDRSTRLEN] = {};

    if (ct_cidr_to_str_r(cidr, cidr_str_x, INET6_ADDRSTRLEN) != 0) {
        return false;
    }

    if (strcmp(cidr_str_x, cidr_str) != 0) {
        return false;
    }

    return true;
}

int main(void) {
    int failed = 0;
    char *cidr_str = nullptr;

    printf("=== Testing cidr ct_sort ===\n");
    /*
     * Unsorted array of IPv4 and IPv6 cidrs
     */
    //CtCidr raw_list[5] = {};
    CtCidrs cidrs = {};

    if (!ct_allocate_cidrs(5, &cidrs)) {
        printf("[FAIL] allocation failed\n");
        return 1;
    }

    /*
     * cidr 0: IPv6 2001:db8:b::/48
     */
    size_t count = 0;

    cidr_str = "2001:db8:b::/48";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[count++]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error on %s\n", cidr_str);
        failed++;
        goto cleanup;
    }

    /*
     * cidr 1: IPv4 192.168.10.0/24
     */
    cidr_str = "192.168.10.0/24";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[count++]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error on %s\n", cidr_str);
        failed++;
        goto cleanup;
    }

    /*
     * cidr 2: IPv4 192.168.1.0/24
     */
    cidr_str = "192.168.1.0/24";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[count++]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error on %s\n", cidr_str);
        failed++;
        goto cleanup;
    }

    /*
     * cidr 3: IPv4 192.168.1.0/16 
     * - same IP part as block 2, but wider prefix. 
     * - Should sort before block 2
     */
    cidr_str = "192.168.1.0/16";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[count++]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error on %s\n", cidr_str);
        failed++;
        goto cleanup;
    }

    /*
     * cidr 4: IPv6 2001:db8:a::/48
     */
    cidr_str = "2001:db8:a::/48";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[count++]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error on %s\n", cidr_str);
        failed++;
        goto cleanup;
    }

    /*
     * Sort em
     */
    if (ct_sort(&cidrs) != 0) {
        printf("[FAIL] Sort failed with valid cidr list.\n");
        failed++;
        goto cleanup;
    }

    printf("[OK] Sorted list has %zu blocks:\n", cidrs.count);
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    for (size_t i = 0; i < cidrs.count; i++) {
        if (ct_cidr_to_str_r(&cidrs.blocks[i], buf, bufsz) != 0) {
            printf("[FAIL] Error making cidr string\n");
            return 1;
        }
        printf("  -> %s\n", buf);
    }

    /*
     * Check some of the sort results.
     * Expecting:
     *  192.168.1.0/16 192.168.1.0/24 192.168.10.0/24 2001:db8:a::/48
     * 
     */
    if (!check_cidr(&cidrs.blocks[0], AF_INET, 16U, "192.168.0.0/16")) {
        failed++;
    }

    if (!check_cidr(&cidrs.blocks[1], AF_INET, 24U, "192.168.1.0/24")) {
        failed++;
    }

    if (!check_cidr(&cidrs.blocks[2], AF_INET, 24U, "192.168.10.0/24")) {
        failed++;
    }

    if (!check_cidr(&cidrs.blocks[3], AF_INET6, 48U, "2001:db8:a::/48")) {
        failed++;
    }

    if (!check_cidr(&cidrs.blocks[4], AF_INET6, 48U, "2001:db8:b::/48")) {
        failed++;
    }

    if (failed == 0) {
        printf("[SUCCESS] cidrs are correctly sorted\n");
    } else {
        printf("[FAIL] cidrs are not correcly sorted\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&cidrs);

    return (failed > 0) ? 1 : 0;
}

