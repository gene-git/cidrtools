/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 *
 * Unit test for forward hostname lookup.
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "cidrtools.h"

int main(void) {
    const char *target_host = "localhost";
    CtCidrs resolved_cidrs = {};
    char ip_str[INET6_ADDRSTRLEN] = {};
    int status = 0;
    size_t i = 0;

    printf("Test: dns forward lookup test on: %s\n", target_host);

    status = ct_hostname_to_address(target_host, &resolved_cidrs);
    if (status != 0) {
        printf("Error: Forward DNS lookup failed for %s\n", target_host);
        return EXIT_FAILURE;
    }

    printf("Lookup successful! Found %zu record(s):\n", resolved_cidrs.count);

    /*
     *  Print results
     */
    for (i = 0; i < resolved_cidrs.count; i++) {
        const char *ntop_status = nullptr;

        switch (resolved_cidrs.blocks[i].addr.family) {
            case AF_INET:
                ntop_status = inet_ntop(AF_INET, &resolved_cidrs.blocks[i].addr.addr.v4, ip_str, sizeof(ip_str));
                break;

            case AF_INET6:
                ntop_status = inet_ntop(AF_INET6, &resolved_cidrs.blocks[i].addr.addr.v6, ip_str, sizeof(ip_str));
                break;

            default:
                printf("  [%zu] Unknown Address Family Type\n", i);
                continue;
        }

        if (ntop_status) {
            printf("  [%zu] %s/%u\n", i, ip_str, resolved_cidrs.blocks[i].prefix);
            
            /*
             * Validate prefixes are sett correctly (32/128)
             */
            if (resolved_cidrs.blocks[i].addr.family == AF_INET && resolved_cidrs.blocks[i].prefix != 32U) {
                printf("  FAIL: IPv4 prefix should be exactly 32\n");
                ct_free_cidrs(&resolved_cidrs);
                return EXIT_FAILURE;
            }
            if (resolved_cidrs.blocks[i].addr.family == AF_INET6 && resolved_cidrs.blocks[i].prefix != 128U) {
                printf("  FAIL: IPv6 prefix should be exactly 128\n");
                ct_free_cidrs(&resolved_cidrs);
                return EXIT_FAILURE;
            }
        } else {
            printf("  Error transforming data to string format\n");
        }
    }

    /*
     * cleanup
     */
    printf("Calling ct_free_cidrs() ...\n");
    ct_free_cidrs(&resolved_cidrs);

    if (resolved_cidrs.blocks == nullptr && resolved_cidrs.count == 0U) {
        printf("Free Memory successful..\n");
        return EXIT_SUCCESS;
    }

    printf("FAIL: Error during cleanup phase\n");
    return EXIT_FAILURE;
}

