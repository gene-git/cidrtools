/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */

#include "cidrtools.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Map a character buffer of cidrs to a CtCidrs struct.
 * The cidrs are separated by a single comma character.
 *
 * i.e. "cidr1,cidr2,cidr2, ..."
 *
 * See also ct_cidrs_to_flat_buffer()
 *
 * In the event there are 1 or more cidr strings which are invalid,
 * they are skipped over rather than returning error.
 * Caller detects this by the returned count being less than the 
 * input count
 *
 * It is much faster for some use cases (e.g. python) than an arrary version.
 *
 * :param flat_str: String with comma separated cidr strings.
 * :param count:  Number of cidrs in the string
 * :param cidrs: The output CtCidrs struct populated the ``count`` cidrs.
 */
int ct_flat_buffer_to_cidrs(const char *flat_str, size_t count, CtCidrs *cidrs) {

    if (!flat_str || !cidrs) {
        return -1;
    }
    
    if (!ct_allocate_cidrs(count, cidrs)) {
        return -1;
    }

    /*
     * zero out the mem so that any trailing unused structs
     * dont contain bad data. Can be unused if bad cidrs get skipped 
     */
    memset(cidrs->blocks, 0, count * sizeof(CtCidr));

    /*
     * Copy input string since strtok modifies,
     */
    char *str_copy = strdup(flat_str);
    if (!str_copy) {
        ct_free_cidrs(cidrs);
        return -1;
    }

    size_t good = 0;
    char *saveptr = nullptr;
    char safe_token_buf[INET6_ADDRSTRLEN + 8];
    size_t safe_token_buf_len = sizeof(safe_token_buf);

    char *token = strtok_r(str_copy, ",", &saveptr);
    
    while (token != nullptr && good < count) {
        if (strlen(token) < safe_token_buf_len) {
            strncpy(safe_token_buf, token, safe_token_buf_len);
            memset(&cidrs->blocks[good], 0, sizeof(CtCidr));

            int rc = ct_str_to_cidr_block(token, &cidrs->blocks[good]);
            if (rc == 0) {
                good++;
            }
        }
        token = strtok_r(nullptr, ",", &saveptr);
    }
    cidrs->count = good;

    free(str_copy);
    return 0;
}

