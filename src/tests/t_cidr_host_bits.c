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

enum {T_PREFIX = 112,};

/*
 * Tests ct_get_host_bits() as it underpins ct_format_host_bits
 * */
int main(void) {
    printf("=== Testing ct_format_host_bits() ===\n");
    int failed = 0;

    /*
     * Test 1: IPv4
     * Input: 192.168.1.50/24 -> Host bits should be 0.0.0.50
     */
    CtCidr v4_cidr;
    v4_cidr.prefix = 24;
    v4_cidr.addr.family = AF_INET;
    inet_pton(AF_INET, "192.168.1.50", &v4_cidr.addr.addr.v4);

    char *v4_host_str = ct_format_host_bits(&v4_cidr);
    if (v4_host_str) {
        if (strcmp(v4_host_str, "0.0.0.50") == 0) {
            printf("[OK] IPv4 host bits isolated cleanly: %s\n", v4_host_str);
        } else {
            printf("[FAIL] Wrong IPv4 host bits extracted. Got: %s\n", v4_host_str);
            failed++;
        }
        free(v4_host_str);
    } else {
        printf("[FAIL] ct_format_host_bits returned nullptr for valid IPv4 block\n");
        failed++;
    }

    /*
     * Test 2: IPv6
     *  Input: 2001:db8::abcd:1234/112 -> The last 16 bits (2 bytes) are host space.
     *  Host bits should evaluate exactly to ::1234
     */
    CtCidr v6_cidr;
    v6_cidr.prefix = T_PREFIX;
    v6_cidr.addr.family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8::abcd:1234", &v6_cidr.addr.addr.v6);

    char *v6_host_str = ct_format_host_bits(&v6_cidr);
    if (v6_host_str) {
        if (strcmp(v6_host_str, "::1234") == 0) {
            printf("[OK] IPv6 host bits isolated cleanly: %s\n", v6_host_str);
        } else {
            printf("[FAIL] Wrong IPv6 host bits extracted. Got: %s\n", v6_host_str);
            failed++;
        }
        free(v6_host_str);
    } else {
        printf("[FAIL] ct_format_host_bits returned nullptr for valid IPv6 block\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

