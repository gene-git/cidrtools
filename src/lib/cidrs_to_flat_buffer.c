/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

/*
 * Max size needed per cidr string plus a comma or null terminator
 */
enum {MAX_ITEM_LEN = INET6_ADDRSTRLEN + 1};

/**
 * Create a single, comma-separated flat string buffer of all CIDRs in CtCidrs
 *
 * The result is malloc() string of the form:
 *
 *  "cidr1,cidr2,cidr3,..."
 *
 * See also ct_flat_buffer_to_cidrs()
 *
 * :param cidrs: The list of cidrs to map to a string.
 * :returns: A dynamically allocated string with the comma separated cidrs or nullptr 
 */
char *ct_cidrs_to_flat_buffer(const CtCidrs *cidrs) {

    if (!cidrs || cidrs->count == 0) {
        return nullptr;
    }

    /*
     * Max total string length needed (using Max of 50 chars per CIDR + comma)
     */
    size_t size_allocated = cidrs->count * MAX_ITEM_LEN; 
    char *flat_buf = malloc(size_allocated);
    if (!flat_buf) {
        return nullptr;
    }

    flat_buf[0] = '\0';
    size_t current_len = 0;
    char one_cidr[MAX_ITEM_LEN];

    for (size_t i = 0; i < cidrs->count; i++) {

        /*
         * Get cidr string for this cidr block
         */
        if (ct_cidr_to_str_r(&cidrs->blocks[i], one_cidr, MAX_ITEM_LEN) != 0) {
            continue;
        }

        size_t one_cidr_len = strlen(one_cidr);
        
        /*
         * Append comma (after first cidr)
         */
        if (i > 0) {
            flat_buf[current_len++] = ',';
        }

        memcpy(&flat_buf[current_len], one_cidr, one_cidr_len);
        current_len += one_cidr_len;
    }
    
    /*
     * Null terminate
     */
    flat_buf[current_len] = '\0'; 

    /*
     * Realloc the size to what was actually used
     */
    size_t size = current_len + 1U;
    if (size < size_allocated) {
        void *ptr = realloc(flat_buf, size);
        if (!ptr) {
            free(flat_buf);
            return nullptr;
        }
        flat_buf = (char *)ptr;
    }

    return flat_buf;
}

