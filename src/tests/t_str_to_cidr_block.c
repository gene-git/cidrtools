/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Test 1: IPv4 CIDR string with prefix
 */
static int test_1() {
    int ret = 0;
    CtCidr cidr = {};
    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    ret = ct_str_to_cidr_block("10.0.0.55/24", &cidr);
    if (ret == 0) {
        if (ct_cidr_to_str_r(&cidr, cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error parsing str to cidr\n");
            return 1;
        }

        if (strcmp(cidr_str, "10.0.0.0/24") == 0) {
            printf("[OK] Success parsed and cleaned dirty string: %s\n", cidr_str);
        } else {
            printf("[FAIL] Failed to parse and clean. Got: %s\n", cidr_str);
            return 1;
        }
    } else {
        printf("[FAIL] Str to CIDR parsing for IPv4\n");
        return 1;
    }

    return 0;
}

/*
 * Test 2: IP string (no prefix) (should get a /32)
 */
static int test_2() {
    int ret = 0;
    CtCidr cidr = {};

    ret = ct_str_to_cidr_block("192.168.1.100", &cidr);
    if (ret == 0) {
        if (cidr.prefix == 32) {
            printf("[OK] IPv4 IP address correctly for prefix /32\n");
        } else {
            printf("[FAIL] IPv4 IP address failed to parse. Got prefix: %u\n", cidr.prefix);
            return 1;
        }
    }

    return 0;
}


/*
 * Test 3: Ip string with no prefix limit
 */
static int test_3() {
    int ret = 0;
    CtCidr cidr = {};
    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    ret = ct_str_to_cidr_block_limit("0.0.0.0/2", &cidr, 0U);
    if (ret == 0) {
        if (ct_cidr_to_str_r(&cidr, cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error parsing str to cidr\n");
            return 1;
        }

        if (strcmp(cidr_str, "0.0.0.0/2") == 0) {
            printf("[OK] Success parsed with no prefix limit: %s\n", cidr_str);
        } else {
            printf("[FAIL] Failed to parse and clean. Got: %s\n", cidr_str);
            return 1;
        }
    } else {
        printf("[FAIL] Str to CIDR parsing for IPv4\n");
        return 1;
    }

    return 0;
}

/*
 * Test 4: Illegal cidr
 * - ct_str_to_cidr_block() should return -2
 * - and the cidr should be reset to 0.0.0.0/32
 */
static int test_4() {
    int ret = 0;
    CtCidr cidr = {};
    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    ret = ct_str_to_cidr_block("goop/2", &cidr);
    if (ret == -2) {
        if (ct_cidr_to_str_r(&cidr, cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] Error parsing str to cidr\n");
            return 1;
        }

        if (strcmp(cidr_str, "0.0.0.0/32") == 0) {
            printf("[OK] Success parsed to reset cidr: %s\n", cidr_str);
        } else {
            printf("[FAIL] Failed to parse and clean. Got: %s\n", cidr_str);
            return 1;
        }
    } else {
        printf("[FAIL] Str to CIDR parsing for IPv4\n");
        return 1;
    }

    return 0;
}


int main(void) {
    int failed = 0;

    printf("=== Testing ct_str_to_cidr_block ===\n");

    failed += test_1();
    failed += test_2();
    failed += test_3();
    failed += test_4();

    return (failed > 0) ? 1 : 0;
}

