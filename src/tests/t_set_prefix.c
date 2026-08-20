/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

enum {BAD_v4_PREFIX = 33,};

int main(void) {
    printf("=== Testing ct_cidr_set_prefix ===\n");
    int failed = 0;

    /*
     * Test 1: Enlarging the cidr block 
     * - (IPv4 /24 -> /16 with host cleansing)
     * - host bits should be cleared
     * - 192.168.1.0/24 ==> 192.168.0.0/16.
     */
    CtCidr block_v4 = { .prefix = 24 };
    block_v4.addr.family = AF_INET;

    if (inet_pton(AF_INET, "192.168.1.0", &block_v4.addr.addr.v4) != 1) {
        return 1;
    }

    char cidr_str[INET_ADDRSTRLEN] = {};
    size_t cidr_str_size = sizeof(cidr_str);

    if (ct_cidr_set_prefix(&block_v4, 16) == 0) {
        //char *s = ct_cidr_to_str(&block_v4);
        if (ct_cidr_to_str_r(&block_v4, cidr_str, cidr_str_size) != 0) {
            printf("[FAIL] ct_cidr_to_str_r error\n");
        }

        if (strcmp(cidr_str, "192.168.0.0/16") == 0) {
            printf("[OK] IPv4 prefix set and host bits updated : %s\n", cidr_str);
        } else {
            printf("[FAIL] Failed to clear host bits. Got: %s\n", cidr_str);
            failed++;
        }
        //free(s);
    } else {
        printf("[FAIL] Set prefix returned error IPv4\n");
        failed++;
    }

    /*
     * Test 2: Illegal Prefix 
     * - Try to pass /33 to an IPv4 block shouldf fail.
     */
    if (ct_cidr_set_prefix(&block_v4, BAD_v4_PREFIX) == -1) {
        printf("[OK] Correcly refused to set bad prefix for IPv4 prefix length\n");
    } else {
        printf("[FAIL] Failed to set invalid prefix\n");
        failed++;
    }

    /*
     * Test 3: Null Pointer Check
     */
    if (ct_cidr_set_prefix(nullptr, 24) == -1) {
        printf("[OK] Correcly rejected a nullptr\n");
    } else {
        printf("[FAIL] Failed to reject nullptr\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

