/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Parses an IP string into a CtAddress structure.
 * String can be any valid numeric IPv4 or IPv6 representation.
 *
 * :param address: The IP address string to parse.
 * :param ip_addr: The resultant CtAddress structure.
 *
 * :returns: 0 on success, or -1 if the string is an invalid IP address.
 */
int ct_str_to_ip_address(const char *address, CtAddress *ip_addr) {
    struct addrinfo hints = {};
    struct addrinfo *res = nullptr;
    const struct sockaddr_in *sin = nullptr;
    const struct sockaddr_in6 *sin6 = nullptr;

    if (!address || !ip_addr) {
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    res = nullptr;
    
    /*
     * AI_NUMERICHOST 
     *  prevents getaddrinfo from trying to hit network DNS 
     *  servers if someone passes an unparsable typo string or a domain name.
     *
     * AF_UNSPEC
     *  Support both IPv4 and IPv6
     */
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST; 

    /*
     * Let glibc parse the IP string 
     */
    if (getaddrinfo(address, nullptr, &hints, &res) != 0) {
        return -1;
    }

    /*
     * Be defensive to ensure got a valid pointer
     */
    if (!res) {
        return -1;
    }

    ip_addr->family = res->ai_family;

    switch (res->ai_family) {
        case AF_INET:
            sin = (const struct sockaddr_in *)res->ai_addr;
            ip_addr->addr.v4 = sin->sin_addr;
            break;

        case AF_INET6:
            sin6 = (const struct sockaddr_in6 *)res->ai_addr;
            ip_addr->addr.v6 = sin6->sin6_addr;
            break;

        default:
            /*
             * Unknown family: Clean up and return error
             */
            freeaddrinfo(res);
            return -1;
    }

    /*
     * Clean up
     */
    freeaddrinfo(res);
    return 0;
}

