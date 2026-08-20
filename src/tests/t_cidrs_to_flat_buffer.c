/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    CtCidrs cidrs = {};
    CtCidrs empty_cidrs = {};
    int failed = 0;
    char *flat_buf = nullptr;

    printf("Running test: t_cidrs_to_flat_buffer...\n");

    /*
     * Initialize cidrs
     */
    const char *cidr_strings[] = {"192.168.0.0/23", "10.10.10.0/24"};
    size_t cidr_strings_count = sizeof(cidr_strings) / sizeof(cidr_strings[0]);

    if (ct_str_array_to_cidrs(cidr_strings, cidr_strings_count, &cidrs) != 0) {
        printf("[FAIL] ct_str_array_to_cidrs initialization failed\n");
        failed++;
        goto cleanup;
    }

    /*
     * sanity check
     */
    if (cidrs.count != cidr_strings_count) {
        printf("[FAIL] wrong count after allocation\n");
        failed++;
        goto cleanup;
    }

    /*
     * Map cidrs to single char buffer
     */
    flat_buf = ct_cidrs_to_flat_buffer(&cidrs);
    if (!flat_buf) {
        printf("[FAIL] wrong count after allocation\n");
        failed++;
        goto cleanup;
    }
    
    /*
     * Check strings ordered and combined correctly with commas
     */
    const char *expected_flat_str = "192.168.0.0/23,10.10.10.0/24";
    if (strcmp(flat_buf, expected_flat_str) != 0) {
        printf("[FAIL] wrong flat buffer\n");
        failed++;
        goto cleanup;
    }

    /*
     * Check empty/edge case works
     */
    char *empty_string = ct_cidrs_to_flat_buffer(&empty_cidrs);
    if (empty_string) {
        printf("[FAIL] empty cidrs returned non-empty string!\n");
        failed++;
        goto cleanup;
    }

    printf("[OK]: t_cidrs_to_flat_buffer passed.\n");

cleanup:
    if (flat_buf) {
        free(flat_buf);
    }
    ct_free_cidrs(&cidrs);
    ct_free_cidrs(&empty_cidrs);

    return failed;
}

