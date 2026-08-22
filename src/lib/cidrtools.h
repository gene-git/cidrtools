/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 *
 * cidrtools
 */
#ifndef CIDRTOOLS_H
#define CIDRTOOLS_H

#include "cidrtools-export.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * A single IP address. It can be either an ``IPv4`` or an ``IPv6`` address.
 *
 * It has two elements:
 *
 * - **family** : One of ``AF_INET`` for ``IPv4`` or ``AF_INET6`` for ``IPv6``.
 *
 * - **add** : This is a union of struct in_addr and struct in6_addr.
 *   The union combines an uint32_t with an array uint8_t[16] in linux.
 */
typedef struct ct_address {
    int family;
    union {
        struct in_addr v4;
        struct in6_addr v6;
    } addr;
} CtAddress;

/**
 * A single CIDR block.
 *
 * A cidr block is an IP Address and a prefix.
 *
 * - **addr** : The IP address
 *
 * - **prefix** : The prefix which is capped at 32 or 128 for ``IPv4`` or ``IPv6``
 */
typedef struct ct_cidr {
    CtAddress addr;
    uint8_t prefix;
} CtCidr;


/**
 * This container struct holds an array of cidr blocks and
 * a count of the number of CtCidr elements in the array.
 *
 * The blocks are allocateed as an array of structs. 
 * To allocate ``count`` cidr blocks :
 *
 *  ct_allocate_cidrs(count, cirs)
 *
 * - **count** : The number of cidrs.
 *
 * - **blocks** : Array ic cidr block structs.
 */
typedef struct ct_cidrs {
    CtCidr *blocks;
    size_t count;
} CtCidrs;


/*
 * Private helper functions not available in shared library symbol tables
 */
int compare_uint8_t(const uint8_t *a, const uint8_t *b);
int compare_uint8_t_qsort(const void *a, const void *b);

void compact_v4(CtCidrs *cidrs);
void compact_v6(CtCidrs *cidrs);

int range_to_cidrs_v4(uint32_t start, uint32_t end, CtCidrs *cidrs);
int range_to_cidrs_v6(const uint8_t *start_bytes, const uint8_t *end_bytes, CtCidrs *cidrs);



/*
 * Public functions available in shared library symbol tables
 */

/*
 * cidr ops
 */
CIDRTOOLS_EXPORT int ct_clean_cidrs(CtCidrs *cidrs); 
CIDRTOOLS_EXPORT int ct_clean_cidr(CtCidr *cidr);
CIDRTOOLS_EXPORT bool ct_cidr_contains_ip(const CtCidr *cidr, const CtAddress *ip);
CIDRTOOLS_EXPORT bool ct_cidr_contains_cidr(const CtCidr *parent, const CtCidr *target);
CIDRTOOLS_EXPORT int ct_cidr_fix_host_bits(CtCidr *cidr);
CIDRTOOLS_EXPORT bool ct_cidr_is_subnet(const CtCidr *cidr, const CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_cidrs_intersection(CtCidrs *cidrs1, CtCidrs *cidrs2, CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_cidr_to_range(const CtCidr *cidr, CtAddress *first, CtAddress *last);
CIDRTOOLS_EXPORT int ct_cidr_to_range_mid(const CtCidr *cidr, CtAddress *first, CtAddress *mid, CtAddress *last);
CIDRTOOLS_EXPORT char *ct_cidr_to_str(const CtCidr *cidr);
CIDRTOOLS_EXPORT int ct_cidr_to_str_r(const CtCidr *cidr, char *buf, size_t buflen);
CIDRTOOLS_EXPORT int ct_str_to_cidr_parts(const char *cidr, char *ip_addr, size_t ip_addr_len, uint8_t *prefix);
CIDRTOOLS_EXPORT int ct_compact(CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_exclude_cidrs(CtCidrs *all, CtCidrs *excluded);
CIDRTOOLS_EXPORT char *ct_format_host_bits(const CtCidr *cidr);
CIDRTOOLS_EXPORT int ct_get_host_bits(const CtCidr *cidr, CtAddress *addr);
CIDRTOOLS_EXPORT bool ct_is_ipv4(const CtCidr *cidr);
CIDRTOOLS_EXPORT bool ct_is_ipv6(const CtCidr *cidr);
CIDRTOOLS_EXPORT size_t ct_num_ips(const CtCidr *cidr);
CIDRTOOLS_EXPORT int ct_range_to_cidrs(const CtAddress *first, const CtAddress *last, CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_cidr_set_prefix(CtCidr *cidr, uint8_t prefix);
CIDRTOOLS_EXPORT int ct_sort(CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_cidr_sort_compare(const void *a, const void *b);
CIDRTOOLS_EXPORT int ct_str_to_cidr_block(const char *str, CtCidr *cidr);

CIDRTOOLS_EXPORT int ct_str_array_to_cidrs(const char **str_array, size_t count, CtCidrs *cidrs);
CIDRTOOLS_EXPORT int ct_cidrs_to_str_array(const CtCidrs *cidrs, char **dest_array);

CIDRTOOLS_EXPORT int ct_flat_buffer_to_cidrs(const char *flat_str, size_t count, CtCidrs *cidrs);
CIDRTOOLS_EXPORT char *ct_cidrs_to_flat_buffer(const CtCidrs *cidrs);

CIDRTOOLS_EXPORT CtCidrs *ct_subnets_split(const CtCidr *cidr, uint8_t prefix);
CIDRTOOLS_EXPORT char *ct_version(void);

CIDRTOOLS_EXPORT int ct_split_by_family(CtCidrs *cidrs, CtCidrs *cidrs_v4, CtCidrs *cidrs_v6);

CIDRTOOLS_EXPORT void ct_free_cidrs(CtCidrs *cidrs);
CIDRTOOLS_EXPORT bool ct_add_cidr_to_cidrs(CtCidrs *cidrs, const CtCidr *cidr);
CIDRTOOLS_EXPORT bool ct_allocate_cidrs(size_t count, CtCidrs *cidrs);
/*
 * ip ops
 */
CIDRTOOLS_EXPORT int ct_ip_address_increment(const CtAddress *addr, size_t num, CtAddress *addr_inc);
CIDRTOOLS_EXPORT int ct_ip_address_range(const CtAddress *addr, uint8_t prefix, CtAddress *first, CtAddress *last);
CIDRTOOLS_EXPORT char *ct_ip_address_to_str(const CtAddress *ip_addr);
CIDRTOOLS_EXPORT int ct_ip_address_to_str_r(const CtAddress *ip_addr, char *buf, size_t buflen);
CIDRTOOLS_EXPORT int ct_str_to_ip_address(const char *address, CtAddress *ip_addr);

/*
 * dns
 */
CIDRTOOLS_EXPORT int ct_ip_str_to_hostname(const char *ip, char *hostname);
CIDRTOOLS_EXPORT int ct_hostname_to_address(const char *hostname, CtCidrs *cidrs);

#endif

