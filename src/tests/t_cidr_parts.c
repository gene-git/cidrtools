/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <stdio.h>

enum {T_BUF_SZ = 49U};


int main() {
    char ip_buffer[T_BUF_SZ] = {}; // Big enough for any IPv6 address
    uint8_t prefix = 0;

    /*
     * Test 1: IPv4 IP Address (no slash) - prefix => /32
     */
    ct_str_to_cidr_parts("192.168.1.50", ip_buffer, sizeof(ip_buffer), &prefix);
    printf("Test 1 -> IP: %s, Prefix: %d\n", ip_buffer, prefix);

    /*
     * Test 2: IPv6 IP Address (no slash) - prefix => 128
     */
    ct_str_to_cidr_parts("2001:db8::1", ip_buffer, sizeof(ip_buffer), &prefix);
    printf("Test 2 -> IP: %s, Prefix: %d\n", ip_buffer, prefix);

    /*
     * Test 3: Standard IPv4 CIDR block /24
     */
    ct_str_to_cidr_parts("10.0.0.0/24", ip_buffer, sizeof(ip_buffer), &prefix);
    printf("Test 3 -> IP: %s, Prefix: %d\n", ip_buffer, prefix);

    return 0;
}

