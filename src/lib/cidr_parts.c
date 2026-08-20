/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Helper - Checks if string has prefix.
 * - if has prefix, then find it and return value in 
 *   *prefix = prefix_value
 *   *slash = nullptr
 *
 * - if no prefix
 *   has_prefix = false
 *   *slash points to the "/" in str
 */
static int parse_prefix(const char *str, const char **slash_p, uint8_t *prefix) {
    const char *slash = nullptr;
    char prefix_buf[16] = {};
    size_t pref_len = 0U;
    long parsed_val = 0U;
    char *endptr = nullptr;

    slash = strchr(str, '/');

    memset(prefix_buf, 0, sizeof(prefix_buf));

    if (!slash) {
        *slash_p = nullptr;

    } else {
        *slash_p = slash;

        /*
         * copy the prefix (it starts after the slash)
         * Note the prefix should have no more than 3 characters (128 is max)
         */
        slash++;
        pref_len = strlen(slash);
        if (pref_len >= sizeof(prefix_buf)) {
            /* 
             * Prefix string is unrealistically long
             */
            return -1;
        }

        memcpy(prefix_buf, slash, pref_len);
        prefix_buf[pref_len] = '\0';

        /*
         * Clean trailing white space
         * by walking backward from the end of prefix_buf
         */
        while (pref_len > 0U && isspace((unsigned char)prefix_buf[pref_len - 1U])) {
            prefix_buf[pref_len - 1U] = '\0';
            pref_len--;
        }

        endptr = nullptr;
        parsed_val = strtol(prefix_buf, &endptr, 10);

        /*
         * If endptr doesn't point to the null terminator, unexpected characters exist
         */
        if (!endptr || *endptr != '\0') {
            return -1; 
        }

        if (parsed_val < 0 || parsed_val > 128) {
            /*
             * Prefix falls completely outside protocol limits
             */
            return -1; 
        }

        *prefix = (uint8_t)parsed_val;
    }
    return 0;
}

/**
 * Splits an input string into an IP address component and a prefix component.
 *
 * If there is no prefix in the input string, prefix will be set to 
 * 32 for ``IPv4`` or 128 for ``IPv6``
 * Caller should then set the prefix based on family.
 *
 * Safely strips any trailing whitespace characters from both prefix and raw IP blocks.
 *
 * :param str: The input CIDR string (e.g., "192.168.1.1/24  \n" or "10.0.0.1  \t\n").
 * :param ip_buf: Output buffer to save the isolated IP string block.
 * :param ip_buflen: Size limit of the destination ip_buf.
 * :param prefix: Output pointer to hold the parsed uint8_t prefix value.
 *
 * :returns: 0 on success, or -1 on formatting errors or overflow boundaries.
 */
int ct_str_to_cidr_parts(const char *str, char *ip_buf, size_t ip_buflen, uint8_t *prefix) {
    const char *slash = nullptr;
    size_t ip_len = 0;

    if (!str || !ip_buf || ip_buflen == 0U || !prefix) {
        return -1;
    }

    if (parse_prefix(str, &slash, prefix) != 0) {
        return -1;
    }

    if (slash) {
        /*
         * prefix provided
         */
        ip_len = (size_t)(slash - str);

    } else {
        /* 
         * No prefix - calculate initial length of full raw string
         */
        ip_len = strlen(str);

        /*
         * FIXED: Strip trailing whitespace when no prefix is provided
         * Step backward from the calculated length to omit trailing spaces/newlines
         */
        while (ip_len > 0U && isspace((unsigned char)str[ip_len - 1U])) {
            ip_len--;
        }

        if (ip_len == 0U) {
            /* 
             * Input string was pure whitespace 
             */
            return -1;
        }

        /*
         * Check protocol family hints based on trimmed boundary boundaries
         */
        if (strchr(str, ':')) {
            *prefix = 128U;

        } else {
            *prefix = 32U;
        }
    }

    if (ip_len >= ip_buflen) {
        /*
         *  IP string length exceeds available provided buffer size
         */
        return -1;
    }

    /*
     * Extract the IP substring
     */
    memcpy(ip_buf, str, ip_len);
    ip_buf[ip_len] = '\0';

    return 0;
}

