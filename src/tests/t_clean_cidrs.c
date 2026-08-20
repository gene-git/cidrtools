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

enum {BAD_v4_PREFIX = 35};

int main(void) {
    int failed = 0;
    char buf[INET6_ADDRSTRLEN] = {};
    size_t bufsz = sizeof(buf);
    char *cidr_str = nullptr;
    const char *good_cidrs[] = {
        "192.168.1.0/24", "2001:db8::/64", "10.1.2.0/24"
    };

    printf("=== Testing CIDR Parsing & CLeaning ===\n");

    /*
     * Test ct_clean_cidrs ---
     */
    CtCidrs cidrs = {};

    if (!ct_allocate_cidrs(3, &cidrs)) {
        printf("[FAIL] memory allocation error\n");
        return 1;
    }
    
    /*
     * Element 0: IPv4 with a bad /35 prefix length
     */
    cidrs.blocks[0].prefix = BAD_v4_PREFIX;
    cidrs.blocks[0].addr.family = AF_INET;
    inet_pton(AF_INET, "192.168.1.255", &cidrs.blocks[0].addr.addr.v4);

    /*
     * Element 1: IPv6 with dirty host bits past it's /64 prefix
     */
    cidrs.blocks[1].prefix = 64;
    cidrs.blocks[1].addr.family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8::ffff", &cidrs.blocks[1].addr.addr.v6);

    /*
     * Element 2: use the library which cleans prefix and host bits 
     * - note it does not modify prefix == 0.
     */
    cidr_str = "10.1.2.3/35";
    if (ct_str_to_cidr_block(cidr_str, &cidrs.blocks[2]) != 0) {
        printf("[FAIL] ct_str_to_cidr_block fails on %s\n", cidr_str);
        failed ++;
        goto cleanup;
    }

    if (ct_clean_cidrs(&cidrs) == 0) {
        for (int i = 0; i < 3; i++) {
            if (ct_cidr_to_str_r(&cidrs.blocks[1], buf, bufsz) != 0) {
                printf("[FAIL] ct_cidr_to_str_r fail\n");
                failed++;
                goto cleanup;
            }
            if (strcmp(buf, good_cidrs[i]) == 0) {
                printf("[OK] Clean success %s\n", buf);
            } else {
                printf("[FAIL] Clean failed: Expect %s got %s\n", good_cidrs[i], buf);

            }
        }
    } else {
        printf("[FAIL] ct_clean_cidrs returned error\n");
        failed++;
    }

cleanup:
    ct_free_cidrs(&cidrs);
    return (failed > 0) ? 1 : 0;
}

