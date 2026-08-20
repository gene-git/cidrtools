/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 *
 * dns reverse lookup
 */
#include "cidrtools.h"
#include <arpa/inet.h>
#include <arpa/nameser.h>           // IWYU pragma: keep
#include <arpa/nameser_compat.h> // IWYU pragma: keep
#include <bits/types/res_state.h>
#include <netinet/in.h>
#include <resolv.h>                 // IWYU pragma: keep
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/**
 * Convenience function to do a direct reverse dns (PTR) lookup of an IP string.
 *
 * :param ip: The ip address to lookup.
 * :param hostname: The fully qualified hostname (FQDN) or empty string if none found.
 *                  hostname must be at least NS_MAXDNAME characters long.
 * :returns: 0 on success, -1 if any issues.
 */
int ct_ip_str_to_hostname(const char *ip, char *hostname) {
    struct in_addr ipv4_addr = {};
    struct in6_addr ipv6_addr = {};
    char query_buffer[256] = {};
    struct __res_state dns_state = {};
    unsigned char response_packet[NS_PACKETSZ] = {};
    int response_len = 0;
    ns_msg handle = {};
    int answer_count = 0;
    char *fqdn = nullptr;
    int i = 0;

    if (!ip || !hostname) {
        return -1;
    }

    /*
     * Format the IP into a reverse DNS lookup string (.arpa)
     */
    if (inet_pton(AF_INET, ip, &ipv4_addr) == 1) {
        /*
         * IPv4
         */
        const unsigned char *bytes = (const unsigned char *)&ipv4_addr.s_addr;
        
        (void)snprintf(query_buffer, sizeof(query_buffer), "%d.%d.%d.%d.in-addr.arpa",
                       bytes[3], bytes[2], bytes[1], bytes[0]);

    } else if (inet_pton(AF_INET6, ip, &ipv6_addr) == 1) {
        /*
         * IPv6
         */
        char *ptr = query_buffer;
        int hi = 0;
        int lo = 0;
        uint8_t byte = 0;

        for (i = 15; i >= 0; i--) {
        
            byte = ipv6_addr.s6_addr[i];
            hi = (int)(((unsigned int)(byte >> 4U)) & 0x0FU);
            lo = (int)(((unsigned int)byte) & 0x0FU);

            ptr += snprintf(ptr, 5, "%x.%x.", lo, hi);
        }
        
        /* 
         * Cast to void to satisfy cert-err33-c
         */
        (void)snprintf(ptr, 10, "ip6.arpa");

    } else {
        /*
         * Unknown or Invalid format
         */
        return -1;
    }

    /*
     *  Initialize a thread-safe resolver struct
     */
    if (res_ninit(&dns_state) != 0) {
        return -1;
    }

    /*
     *  Query DNS directly for PTR record
     */
    response_len = res_nquery(&dns_state, query_buffer, C_IN, T_PTR,
                              response_packet, sizeof(response_packet));

    if (response_len < 0) {
        /*
         * Error or no record found
         */
        res_nclose(&dns_state);
        return -1; 
    }

    /*
     *  Parse the returned payload
     */
    if (ns_initparse(response_packet, response_len, &handle) < 0) {
        res_nclose(&dns_state);
        return -1;
    }

    answer_count = ns_msg_count(handle, ns_s_an);

    for (i = 0; i < answer_count; i++) {
        ns_rr record;
        memset(&record, 0, sizeof(record));

        if (ns_parserr(&handle, ns_s_an, i, &record) < 0) {
            continue;
        }

        if (ns_rr_type(record) == T_PTR) {
            fqdn = hostname;

            /*
             * Expand compressed DNS pointer format into a readable string
             */
            if (ns_name_uncompress(ns_msg_base(handle), ns_msg_end(handle),
                                   ns_rr_rdata(record), fqdn, NS_MAXDNAME) < 0) {
                continue;
            }
            break;
        }
    }

    res_nclose(&dns_state);
    return 0;
}

