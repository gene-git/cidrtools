/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_ip_struct(const char *label, const CtAddress *ip) {
    char buf[INET6_ADDRSTRLEN];
    size_t bufsz = sizeof(buf);

    buf[0] = '\0';
    (void)ct_ip_address_to_str_r(ip, buf, bufsz);

    printf("%s: %s\n", label, buf);
}

int main() {
    CtCidr cidr = {};
    CtAddress first_ip = {};
    CtAddress mid_ip = {};
    CtAddress last_ip = {};
    char *cidr_str = nullptr;

    /*
     * Test 1: IPv4 Subnet (192.168.1.50 /24)
     *  (192.168.1.50 /24) => 
     *      - 192.168.1.0
     *      - 192.168.1.127
     *      - 192.168.1.255
     */
    cidr_str = "192.168.1.50/24";
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }

    if (ct_cidr_to_range_mid(&cidr, &first_ip, &mid_ip, &last_ip) == 0) {
        printf("--- IPv4 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);

        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip); // Expected: 192.168.1.0
        print_ip_struct("mid", &mid_ip);     // Expected: 192.168.1.127
        print_ip_struct("Last ", &last_ip);  // Expected: 192.168.1.255
    }

    /*
     * Test 2: IPv6 Subnet 
     *  (2001:db8:abcd:1234::55 /56) => 
     *      - 2001:db8:abcd:1200::
     *      - 2001:db8:abcd:127f:8000:: 
     *      - 2001:db8:abcd:12ff:ffff:ffff:ffff:ffff)
     */
    memset(&cidr, 0, sizeof(CtCidr));
    cidr_str = "2001:db8:abcd:1234::55/56";
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }
    
    if (ct_cidr_to_range_mid(&cidr, &first_ip, &mid_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("mid ", &mid_ip);
        print_ip_struct("Last ", &last_ip);
    }

    /*
     * Test 3: IPv6 Subnet 
     *  (2001:db8::/64) => 
     *      - 2001:db8::
     *      - 2001:db8:0:0:8000:0:0:0 ?? 2001:db8:0:7f:8000::
     *      - 2001:db8:0:ff:ffff:ffff:ffff:ffff
     */
    memset(&cidr, 0, sizeof(CtCidr));
    cidr_str = "2001:db8::/64";
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }
    
    if (ct_cidr_to_range_mid(&cidr, &first_ip, &mid_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("mid ", &mid_ip);
        print_ip_struct("Last ", &last_ip);
    }

    /*
     * Test 4 : /127 Subnet
     */
    memset(&cidr, 0, sizeof(CtCidr));
    cidr_str = "2001:db8::abcd:1234/127";
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }

    if (ct_cidr_to_range_mid(&cidr, &first_ip, &mid_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("mid ",  &mid_ip);
        print_ip_struct("Last ", &last_ip);
    }

    /*
     * Test 5: /128 Subnet (Single Host)
     */
    memset(&cidr, 0, sizeof(CtCidr));
    cidr_str = "2001:db8::ffff:ffff/128";
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }

    if (ct_cidr_to_range_mid(&cidr, &first_ip, &mid_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("mid ",  &mid_ip);
        print_ip_struct("Last ", &last_ip);
    }


    return 0;
}

