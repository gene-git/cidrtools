/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include <arpa/nameser.h>
#include <stdio.h>
#include <string.h>

#include "cidrtools.h"

int main(void) {
    printf("=== Testing ct_ip_str_to_hostname ===\n");
    int failed = 0;
    const char *ipv4 = "172.253.139.101";
    const char *ipv4_hostname = "yuiadsk-in-f101.1e100.net";
    const char *ipv6 = "2607:f8b0:4004:c1f::65";
    const char *ipv6_hostname = "pd-in-f101.1e100.net";
    char hostname[NS_MAXDNAME] = {};


    /*
     *  Test 1: IPv4 lookup
     */
    if (ct_ip_str_to_hostname(ipv4, hostname) == 0) {
        if (strcmp(hostname, ipv4_hostname) == 0) {
            printf("[OK] DNS PTR IPv4 correct: %s\n", hostname);
        } else {
            printf("[FAIL] ct_ip_str_to_hostname IPv4\n");
            printf("     Want: %s Got %s\n", ipv4_hostname, hostname);
        }
    } else {
        printf("[FAIL] ct_ip_str_to_hostname IPv4 => error\n");
        failed++;
    }

    /*
     *  Test 2: IPv6 lookup
     */
    if (ct_ip_str_to_hostname(ipv6, hostname) == 0) {
        if (strcmp(hostname, ipv6_hostname) == 0) {
            printf("[OK] DNS PTR IPv6 correct: %s\n", hostname);
        } else {
            printf("[FAIL] ct_ip_str_to_hostname IPv6\n");
            printf("     Want: %s Got %s\n", ipv6_hostname, hostname);
        }
    } else {
        printf("[FAIL] ct_ip_str_to_hostname IPv6 => error\n");
        failed++;
    }

    return (failed > 0) ? 1 : 0;
}

