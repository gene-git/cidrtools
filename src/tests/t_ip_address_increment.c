/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== Testing ct_ip_address_increment ===\n");
    int failed = 0;
    char ip_str[INET6_ADDRSTRLEN] = {};
    size_t ip_str_size = sizeof(ip_str);

    /*
     * Test 1: IPv4 Multi-Step Increment
     */
    CtAddress start_v4 = {};
    CtAddress inc_v4 = {};

    if (ct_str_to_ip_address("192.168.1.250", &start_v4) != 0) {
        printf("[FAIL] Error in ct_str_to_ip_address\n");
        return 1;
    }

    if (ct_ip_address_increment(&start_v4, 5U, &inc_v4) == 0) {

        if (ct_ip_address_to_str_r(&inc_v4, ip_str, ip_str_size) != 0) {
            printf("[FAIL] error from ct_ip_address_to_str_r\n");
        }

        if (strcmp(ip_str, "192.168.1.255") == 0) {
            printf("[OK] Successfully incremented IPv4 to: %s\n", ip_str);
        } else {
            printf("[FAIL] Bad IPv4 incremented result: %s\n", ip_str);
            failed++;
        }
    }

    /*
     * Test 2: IPv6 - step over the last ip in cidr block
     * - lets really push this hard.
     * - last byte is 255 (00ff)
     * - increment by 1 pushes 00ff -> 0100
     */
    CtAddress start_v6 = {};
    CtAddress inc_v6 = {};

    if (ct_str_to_ip_address("2001:db8::00ff", &start_v6) != 0) {
        printf("[FAIL] Error in ct_str_to_ip_address\n");
        return 1;
    }
    
    /*
     * Incrementing by 1 should clear the low byte and step the next byte up to 1
     */
    if (ct_ip_address_increment(&start_v6, 1U, &inc_v6) == 0) {

        if (ct_ip_address_to_str_r(&inc_v6, ip_str, ip_str_size) != 0) {
            printf("[FAIL] error from ct_ip_address_to_str_r\n");
        }

        if (strcmp(ip_str, "2001:db8::100") == 0) {
            printf("[OK] Successfully incremented IPv6 to: %s\n", ip_str);
        } else {
            printf("[FAIL] Bad IPv6 result: %s\n", ip_str);
            failed++;
        }
    }

    return (failed > 0) ? 1 : 0;
}

