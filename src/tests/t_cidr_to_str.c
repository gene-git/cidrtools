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
    printf("=== Testing ct_cidr_to_str ===\n");
    int failed = 0;
    char *cidr_str = nullptr;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);

    /*
     * Test 1: IPv4 CIDR 
     */
    CtCidr v4_cidr = {};

    cidr_str = "10.0.0.0/24";
    if (ct_str_to_cidr_block(cidr_str, &v4_cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block returns error\n");
        return 1;
    }
   
    buf[0] = '\0';
    if (ct_cidr_to_str_r(&v4_cidr, buf, bufsz) != 0) {
        printf("[FAIL] Error from ct_cidr_to_str_r\n");
        return 1;
    }

    if (buf[0] == '\0') {
        printf("[FAIL] ct_cidr_to_str_r got empty string for valid IPv4 block\n");
        failed++;
    } else {
        if (strcmp(buf, cidr_str) == 0) {
            printf("[OK] IPv4 CIDR string matched: %s\n", buf);
        } else {
            printf("[FAIL] IPv4 CIDR string mismatched. Got: %s\n", buf);
            failed++;
        }
    }

    /*
     * Test 2: IPv6 CIDR 
     */
    CtCidr v6_cidr = {};

    cidr_str = "fd00::/64";
    if (ct_str_to_cidr_block(cidr_str, &v6_cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block returns error\n");
        return 1;
    }
   
    buf[0] = '\0';
    if (ct_cidr_to_str_r(&v6_cidr, buf, bufsz) != 0) {
        printf("[FAIL] Error from ct_cidr_to_str_r\n");
        return 1;
    }
    if (buf[0] == '\0') {
        printf("[FAIL] ct_cidr_to_str_r got empty string for valid IPv4 block\n");
        failed++;

    } else {
        if (strcmp(buf, cidr_str) == 0) {
            printf("[OK] IPv6 CIDR string matched: %s\n", buf);
        } else {
            printf("[FAIL] IPv6 CIDR string mismatched. Got: %s\n", buf);
            failed++;
        }
    }

    return (failed > 0) ? 1 : 0;
}

