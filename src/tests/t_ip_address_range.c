/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>

static void print_ip_struct(const char *label, const CtAddress *ip) {
    char buf[INET6_ADDRSTRLEN];
    size_t bufsz = sizeof(buf);

    buf[0] = '\0';
    (void)ct_ip_address_to_str_r(ip, buf, bufsz);
    printf("%s: %s\n", label, buf);
}

/*
 * Legacy test: Superceded by ct_cidr_to_range()
 */
int main() {
    CtAddress base_ip;
    CtAddress first_ip;
    CtAddress last_ip;
    uint8_t prefix = 0;
    char *ip_str = nullptr; 

    printf("=== Testing ct_ip_address_range ==\n");

    /*
     * Test 1: IPv4 Subnet (192.168.1.50 /24)
     * - first: 192.168.1.0
     * - last : 192.168.1.255
     */ 
    ip_str = "192.168.1.50";

    if (ct_str_to_ip_address(ip_str, &base_ip) != 0) {
        printf("[FAIL] ct_str_to_ip_address returned error\n");
        return 1;
    }

    prefix = 24U;
    if (ct_ip_address_range(&base_ip, prefix, &first_ip, &last_ip) == 0) {
        printf("--- IPv4 Range Test ---\n");
        printf("prefix: %u\n", prefix);
        print_ip_struct("Input", &base_ip);
        print_ip_struct("First", &first_ip);
        print_ip_struct("Last ", &last_ip);
    }

    /*
     * Test 2: IPv6 Subnet (2001:db8:abcd:1234::55 /56)
     * - first: 2001:db8:abcd:1200::
     * - last : 2001:db8:abcd:12ff:ffff:ffff:ffff:ffff
     */
    ip_str = "2001:db8:abcd:1234::55";
    if (ct_str_to_ip_address(ip_str, &base_ip) != 0) {
        printf("[FAIL] ct_str_to_ip_address returned error\n");
        return 1;
    }

    prefix = 56U;
    if (ct_ip_address_range(&base_ip, prefix, &first_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", prefix);
        print_ip_struct("Input", &base_ip);
        print_ip_struct("First", &first_ip);
        print_ip_struct("Last ", &last_ip);
    }

    return 0;
}

