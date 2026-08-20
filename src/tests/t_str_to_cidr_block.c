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
    printf("=== Testing ct_str_to_cidr_block ===\n");
    int failed = 0;

    CtCidr block1 = {};
    CtCidr block2 = {};
    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    /*
     * Test 1: IPv4 CIDR string with prefix
     */
    if (ct_str_to_cidr_block("10.0.0.55/24", &block1) == 0) {
        if (ct_cidr_to_str_r(&block1, cidr_str,cidr_str_size) != 0) {
            printf("[FAIL] Error parsing str to cidr\n");
            return 1;
        }

        if (strcmp(cidr_str, "10.0.0.0/24") == 0) {
            printf("[OK] Success parsed and cleaned dirty string: %s\n", cidr_str);
        } else {
            printf("[FAIL] Failed to parse and clean. Got: %s\n", cidr_str);
            failed++;
        }
    } else {
        printf("[FAIL] Str to CIDR parsing for IPv4\n");
        failed++;
    }

    /*
     * Test 2: IP string (no prefix) (should get a /32)
     */
    if (ct_str_to_cidr_block("192.168.1.100", &block2) == 0) {
        if (block2.prefix == 32) {
            printf("[OK] IPv4 IP address correctly for prefix /32\n");
        } else {
            printf("[FAIL] IPv4 IP address failed to parse. Got prefix: %u\n", block2.prefix);
            failed++;
        }
    }
    return (failed > 0) ? 1 : 0;
}

