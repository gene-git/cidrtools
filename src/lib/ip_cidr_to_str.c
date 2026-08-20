/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Format an IP address into a caller-supplied buffer.
 * Eliminates heap allocation bottlenecks inside high-frequency execution tracks.
 *
 * See also ct_ip_address_to_str() which allocates the buffer
 *
 * :param ip_addr: The IP address to parse.
 * :param buf: Destination buffer (must be at least INET6_ADDRSTRLEN bytes long).
 * :param buflen: Size of provided buffer.
 *
 * :returns: 0 on success, or -1 on failure.
 */
int ct_ip_address_to_str_r(const CtAddress *ip_addr, char *buf, size_t buflen) {
    const char *result = nullptr;

    if (!ip_addr || !buf || buflen == 0U) {
        return -1;
    }

    result = nullptr;

    switch (ip_addr->family) {
        case AF_INET:
            if (buflen >= INET_ADDRSTRLEN) {
                result = inet_ntop(AF_INET, &ip_addr->addr.v4, buf, (socklen_t)buflen);
            }
            break;

        case AF_INET6:
            if (buflen >= INET6_ADDRSTRLEN) {
                result = inet_ntop(AF_INET6, &ip_addr->addr.v6, buf, (socklen_t)buflen);
            }
            break;

        default:
            break;
    }

    if (!result) {
        return -1;
    }

    return 0;
}

/**
 * Format a CIDR block into a caller-supplied buffer.
 *
 * See also ct_cidr_to_str() which allocates a buffer.
 *
 * :param cidr: The CIDR to generate the string from.
 * :param buf: Buffer for the result
 * :param buflen: Buffer size. For ``IPv6``, minimum is 49 bytes for ``IPv4`` 19 bytes
 *
 * :returns: 0 on success, or -1 on failure.
 */
int ct_cidr_to_str_r(const CtCidr *cidr, char *buf, size_t buflen) {
    char ip_buf[INET6_ADDRSTRLEN] = {};
    int written = 0;

    if (!cidr || !buf || buflen == 0U) {
        return -1;
    }

    if (ct_ip_address_to_str_r(&cidr->addr, ip_buf, sizeof(ip_buf)) != 0) {
        return -1;
    }

    written = snprintf(buf, buflen, "%s/%u", ip_buf, cidr->prefix);
    if (written < 0 || (size_t)written >= buflen) {
        return -1;
    }

    return 0;
}

/**
 * Return a formated string of the provided IP address.
 *
 * The string is malloc()'ed and the caller is responsible for 
 * calling free() when non-nullptr.
 *
 * See also ct_ip_address_to_str_r() where caller provides the buffer for ouput.
 *
 *  * Example of a returned string: "10.0.0.22"
 *
 * ``IP`` can be ``IPv4`` or ``IPv6``.
 *
 * Note: caller is responsible for free()ing the returned pointer.
 *
 * :param ip_addr: The ip address to get in string form.
 * :returns: A malloc'ed string of the ip address. nullptr if non-IP or empty imput.
 */
char *ct_ip_address_to_str(const CtAddress *ip_addr) {
    size_t alloc_len = 0U;
    char *buf = nullptr;

    if (!ip_addr) {
        return nullptr;
    }

    alloc_len = (ip_addr->family == AF_INET6) ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
    buf = malloc(alloc_len);
    if (!buf) {
        return nullptr;
    }

    if (ct_ip_address_to_str_r(ip_addr, buf, alloc_len) != 0) {
        free(buf);
        return nullptr;
    }

    return buf;
}

/**
 * Returns a formatted string of the CIDR block.
 *
 * Result is malloc()'ed and caller must free().
 * See also ct_cidr_to_str_r() which uses caller provided buffer.
 *
 * Cidr can be ``IPv4`` or ``IPv6``.
 *
 * Example of a returned string: "10.0.0.9./24"
 *
 * Note: The caller is responsible for using free() on the returned pointer.
 *
 * :param cidr: The ip address to get in string form.
 *
 * :returns: A malloc'ed string of the ip address. nullptr if non-IP or empty imput.
 */

char *ct_cidr_to_str(const CtCidr *cidr) {
    char ip_buf[INET6_ADDRSTRLEN] = {};
    size_t total_len = 0;
    char *cidr_str = nullptr;

    if (!cidr) {
        return nullptr;
    }

    if (ct_ip_address_to_str_r(&cidr->addr, ip_buf, sizeof(ip_buf)) != 0) {
        return nullptr;
    }

    /*
     * IP len + '/' + max 3 digits for prefix + null-terminator
     */
    total_len = strlen(ip_buf) + 1U + 3U + 1U;
    cidr_str = malloc(total_len);
    if (!cidr_str) {
        return nullptr;
    }

    if (ct_cidr_to_str_r(cidr, cidr_str, total_len) != 0) {
        free(cidr_str);
        return nullptr;
    }

    return cidr_str;
}

