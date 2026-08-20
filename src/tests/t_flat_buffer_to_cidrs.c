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
    /*
     * Input data 
     * - list of 4 cidrs - 3 good and one bad.
     */
    const char *flat_input = "10.0.0.0/24,192.168.1.0/24,10.111.12.13.14,172.16.0.0/16";
    size_t count = 4;
    const size_t expected_count = count - 1U;
    char cidr_str[INET6_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);
    int failed = 0;

    printf("Running test: ct_flat_buffer_to_cidrs ...\n");

    /*
     * Map the flat buffer into CtCidrs
     */
    CtCidrs cidrs = {};
    if (ct_flat_buffer_to_cidrs(flat_input, count, &cidrs) != 0) {
        printf("[FAIL] ct_flat_buffer_to_cidrs returned error\n");
        failed++;
        goto cleanup;
    }
    
    /*
     * Check all went well
     */
    if (cidrs.count != expected_count) {
        printf("[FAIL] Wrong number of cidrs expected %zu got %zu\n", expected_count, cidrs.count);
        failed++;
        goto cleanup;
    }

    if (!cidrs.blocks) {
        printf("[FAIL] no valid cidrs returned\n");
        failed++;
        goto cleanup;
    }

    /*
     * Verify a couple of individual cidr blocks 
     */
    if (ct_cidr_to_str_r(&cidrs.blocks[0], cidr_str, cidr_str_size) != 0) {
        printf("[FAIL] Error from ct_cidr_to_str_r\n");
        failed++;
        goto cleanup;
    }

    if (strcmp(cidr_str, "10.0.0.0/24") != 0) {
        printf("[FAIL] wrong cidr\n");
        failed++;
        goto cleanup;
    }

    if (ct_cidr_to_str_r(&cidrs.blocks[2], cidr_str, cidr_str_size) != 0) {
        printf("[FAIL] Error from ct_cidr_to_str_r\n");
        failed++;
        goto cleanup;
    }
    if (strcmp(cidr_str, "172.16.0.0/16") != 0) {
        printf("[FAIL] wrong cidr\n");
        failed++;
        goto cleanup;
    }

    /* 
     * Check invalid cidr string 
     * - this is not an error, it returns only the good ones
     */
    CtCidrs bad_cidrs = {};
    if (ct_flat_buffer_to_cidrs("10.0.0.0/24,10.0.0.0/invalid_mask", 2, &bad_cidrs) != 0) {
        printf("[FAIL] Error using bad cidr string with ct_flat_buffer_to_cidrs\n");
        failed++;
        goto cleanup;
    }

    if (bad_cidrs.count != 1) {
        printf("[FAIL] Wrong number cidrs returned\n");
        failed++;
        goto cleanup;

    }

    printf("SUCCESS: ct_flat_buffer_to_cidrs all is good.\n");

cleanup:
    ct_free_cidrs(&cidrs);
    ct_free_cidrs(&bad_cidrs);

    return failed;
}

