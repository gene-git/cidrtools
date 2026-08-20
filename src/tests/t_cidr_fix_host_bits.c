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
    printf("=== Testing ct_cidr_fix_host_bits() - ficing dirty CIDRS===\n");
    int failed = 0;

    /*
     * Test 1: IPv4
     *  192.168.1.50/24 -> Host bits (.50) 192.168.1.0/24
     */
    CtCidr v4_block = { .prefix = 24 };
    v4_block.addr.family = AF_INET;

    if (inet_pton(AF_INET, "192.168.1.50", &v4_block.addr.addr.v4) != 1) {
        return 1;
    }

    if (ct_cidr_fix_host_bits(&v4_block) == 0) {
        char *s = ct_cidr_to_str(&v4_block);
        if (strcmp(s, "192.168.1.0/24") == 0) {
            printf("[OK] IPv4 dirty host bits zeroed perfectly: %s\n", s);
        } else {
            printf("[FAIL] Failed to clean IPv4 host bits. Got: %s\n", s);
            failed++;
        }
        free(s);
    } else {
        printf("[FAIL] Function returned an internal error code for IPv4.\n");
        failed++;
    }

    /* 
     * Test 2: IPv6
     *  2001:db8:abcd:1234::ff/56 -> Host bits 56th => 0
     *  56 bits = exactly 7 bytes ("2001:db8:abcd:12"). 
     *  The 8th byte ("34") should become "00".
     */
    CtCidr v6_block = { .prefix = 56 };
    v6_block.addr.family = AF_INET6;

    if (inet_pton(AF_INET6, "2001:db8:abcd:1234::ff", &v6_block.addr.addr.v6) != 1) {
        return 1;
    }

    if (ct_cidr_fix_host_bits(&v6_block) == 0) {
        char *s = ct_cidr_to_str(&v6_block);
        if (strcmp(s, "2001:db8:abcd:1200::/56") == 0) {
            printf("[OK] IPv6 dirty host bits zeroed perfectly: %s\n", s);
        } else {
            printf("[FAIL] Failed to clean IPv6 host bits. Got: %s\n", s);
            failed++;
        }
        free(s);
    } else {
        printf("[FAIL] Function returned an internal error code for IPv6.\n");
        failed++;
    }

    /*
     * Test 3: Null Safety Check
     */
    if (ct_cidr_fix_host_bits(nullptr) == -1) {
        printf("[OK] Gracefully rejected nullptr input target\n");
    } else {
        printf("[FAIL] Failed nullptr pointer validation constraint safety check\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

