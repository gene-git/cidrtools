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

enum {BAD_V4_PREFIX = 35};

int main(void) {
    printf("=== Testing CIDR ct_cidr_clean() ===\n");
    int failed = 0;
    CtCidr cidr_v4 = {};
    CtCidr cidr_v6 = {};

    /*
     * cidr_v4 : Bad prefix: IPv4 with a bad /35 prefix length
     */
    cidr_v4.prefix = BAD_V4_PREFIX;
    cidr_v4.addr.family = AF_INET;
    inet_pton(AF_INET, "192.168.1.255", &cidr_v4.addr.addr.v4);

    /*
     * cidr_v6 : IPv6 with dirty host bits past a /64 boundary
     */
    cidr_v6.prefix = 64;
    cidr_v6.addr.family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8::ffff", &cidr_v6.addr.addr.v6);

    /*
     * Clean them.
     */
    if (ct_clean_cidr(&cidr_v4) == 0 && ct_clean_cidr(&cidr_v6) == 0) {
        char *s0 = ct_cidr_to_str(&cidr_v4);
        char *s1 = ct_cidr_to_str(&cidr_v6);

        if (cidr_v4.prefix == 32 && strcmp(s1, "2001:db8::/64") == 0) {
            printf("[OK] ct_clean_cidr fixed bad prefix and zerod host bits\n");
        } else {
            printf("[FAIL] Failed fixing: bad prefix or host bits: %s and %s\n", s0, s1);
            failed++;
        }
        free(s0);
        free(s1);
    } else {
        printf("[FAIL] ct_clean_cidr returned an error\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

