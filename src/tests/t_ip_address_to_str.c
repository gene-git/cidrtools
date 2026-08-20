/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int main(void) {
    printf("=== Testing ct_ip_address_to_str ===\n");
    int failed = 0;

    /*
     * Test 1: IPv4 
     */
    CtAddress v4_addr = { .family = AF_INET };
    if (inet_pton(AF_INET, "192.168.23.45", &v4_addr.addr.v4) != 1) {
        printf("[FAIL] IPv4 setup failed\n");
        return 1;
    }

    char *v4_str = ct_ip_address_to_str(&v4_addr);
    if (!v4_str) {
        printf("[FAIL] ct_ip_address_to_str returned nullptr for valid IPv4\n");
        failed++;
    } else {
        if (strcmp(v4_str, "192.168.23.45") == 0) {
            printf("[OK] IPv4 matched: %s\n", v4_str);
        } else {
            printf("[FAIL] IPv4 mismatched. Got: %s\n", v4_str);
            failed++;
        }
        free(v4_str);
    }

    /*
     * Test 2: IPv6 
     */
    CtAddress v6_addr = { .family = AF_INET6 };
    if (inet_pton(AF_INET6, "2001:db8::1", &v6_addr.addr.v6) != 1) {
        printf("[FAIL] IPv6 setup failed\n");
        return 1;
    }

    char *v6_str = ct_ip_address_to_str(&v6_addr);
    if (!v6_str) {
        printf("[FAIL] ct_ip_address_to_str returned nullptr for valid IPv6\n");
        failed++;
    } else {
        /*
         * System libraries generate lowercase compressed (0:0.. -> ::) strings
         */
        if (strcmp(v6_str, "2001:db8::1") == 0) {
            printf("[OK] IPv6 matched: %s\n", v6_str);
        } else {
            printf("[FAIL] IPv6 mismatched. Got: %s\n", v6_str);
            failed++;
        }
        free(v6_str);
    }

    /*
     * Test 3: Error Check
     */
    char *null_check = ct_ip_address_to_str(nullptr);
    if (null_check == NULL) {
        printf("[OK] Correctly rejected nullptr\n");
    } else {
        printf("[FAIL] Did not reject nullptr on input\n");
        failed++;
        free(null_check);
    }

    return (failed > 0) ? 1 : 0;
}

