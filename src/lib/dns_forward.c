/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Performs a forward DNS lookup for a hostname and populates a CtCidrs list.
 * Each returned address block will have its network prefix explicitly set to 
 * 32 (for IPv4) or 128 (for IPv6).
 *
 * The caller is responsible for passing an initialized, clean CtCidrs structure
 * where blocks can be dynamically allocated.
 *
 * :param hostname: The string domain/hostname to look up.
 * :param cidrs: Pointer to the destination CtCidrs structure.
 *
 * :returns: 0 on success, or -1 on failure/invalid resolutions.
 */
int ct_hostname_to_address(const char *hostname, CtCidrs *cidrs) {
    struct addrinfo hints = {};
    struct addrinfo *res = nullptr;
    struct addrinfo *curr = nullptr;
    size_t count = 0U;
    CtCidr *allocated_blocks = nullptr;
    size_t i = 0U;
    const struct sockaddr_in *sin = nullptr;
    const struct sockaddr_in6 *sin6 = nullptr;

    if (!hostname || !cidrs) {
        return -1;
    }

    cidrs->blocks = nullptr;
    cidrs->count = 0U;

    /*
     * Initialize - allow IPv4 and IPv6
     */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    res = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        return -1;
    }

    if (!res) {
        return -1;
    }

    /*
     * Count the number of valid network records returned
     */
    count = 0U;
    curr = res;
    while (curr) {
        if (curr->ai_family == AF_INET || curr->ai_family == AF_INET6) {
            count++;
        }
        curr = curr->ai_next;
    }

    if (count == 0U) {
        freeaddrinfo(res);
        return -1;
    }

    /*
     * Allocate memory needed in output 
     */
    allocated_blocks = malloc(count * sizeof(CtCidr));
    if (!allocated_blocks) {
        freeaddrinfo(res);
        return -1;
    }

    /*
     * Extract data bytes and set prefixes
     */
    curr = res;
    i = 0U;
    while (curr && i < count) {
        switch (curr->ai_family) {
            case AF_INET:
                sin = (const struct sockaddr_in *)curr->ai_addr;
                allocated_blocks[i].addr.family = AF_INET;
                allocated_blocks[i].addr.addr.v4 = sin->sin_addr;
                allocated_blocks[i].prefix = 32U;
                i++;
                break;

            case AF_INET6:
                sin6 = (const struct sockaddr_in6 *)curr->ai_addr;
                allocated_blocks[i].addr.family = AF_INET6;
                allocated_blocks[i].addr.addr.v6 = sin6->sin6_addr;
                allocated_blocks[i].prefix = 128U;
                i++;
                break;

            default:
                /* 
                 * Skip unknown addressing
                 */
                break;
        }
        curr = curr->ai_next;
    }

    /*
     * Clean up mem we allocated
     */
    freeaddrinfo(res);

    cidrs->blocks = allocated_blocks;
    cidrs->count = i;

    return 0;
}

