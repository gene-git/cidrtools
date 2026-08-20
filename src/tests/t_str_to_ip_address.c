/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>


int main() {
    CtAddress ip = {};

    printf(" Test ct_str_to_ip_address\n");

    /* 
     * Test 1: IPv4
     */
    if (ct_str_to_ip_address("192.168.1.25", &ip) == 0) {
        printf("[OK]: Success parsing IPv4 address. Got family flag: %d\n", ip.family);
        printf("      Raw decimal data: %u\n", ip.addr.v4.s_addr);
    }

    /*
     * Test 2: IPv6 
     * - print raw bytes of raw data
     */
    if (ct_str_to_ip_address("2001:db8::1", &ip) == 0) {
        printf("[OK] Success parsing IPv6. Got family flag: %d\n", ip.family);
        printf("     Raw hex data: %x.%x.%x.%x\n", 
                ip.addr.v6.s6_addr[0], 
                ip.addr.v6.s6_addr[1],
                ip.addr.v6.s6_addr[2],
                ip.addr.v6.s6_addr[3]
                );
    }

    /*
     * Test 3: Bogus string 
     */
    if (ct_str_to_ip_address("999.abc.1.1", &ip) != 0) {
        printf("[OK] Correctly rejected invalid IP string.\n");
    }

    return 0;
}

