/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <endian.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>


static unsigned int count_trailing_zeros_v6(const uint8_t *addr) {
    uint64_t low = 0U;
    uint64_t high =0U;

    /*
     * Extract extract upper and lower 64-bit words from the byte array
     * (Handles any alignment issues transparently via memcpy)
     */
    memcpy(&high, addr, 8);
    memcpy(&low, addr + 8, 8);

    /*
     * Swap big-endian network bytes to native host architecture byte order
     * low = __builtin_bswap64(low);
     * high = __builtin_bswap64(high);
     */
    low = be64toh(low);
    high = be64toh(high);

    /*
     * Protect __builtin_ctzllfrom UB with 0
     */
    if (low != 0U) {
        return (unsigned int)__builtin_ctzll(low);
    }

    if (high != 0U) {
        return 64U + (unsigned int)__builtin_ctzll(high);
    }

    return 128U;
}

static void add_v6_inplace(uint8_t *addr, unsigned int bits) {
    uint64_t low = 0U;
    uint64_t high =0U;

    /*
     *  Unpack big-endian structures
     */
    memcpy(&high, addr, 8);
    memcpy(&low, addr + 8, 8);

    /*
     * high = __builtin_bswap64(high);
     * low = __builtin_bswap64(low);
     */
    high = be64toh(high);
    low = be64toh(low);

    if (bits >= 64U) {
        /* 
         * addition is within the upper 64 bits
         * If bits == 128U, it overflows the 128-bit space; do nothing
         */
        if (bits < 128U) {
            high += (uint64_t)1 << (bits - 64U);
        }

    } else {
        /*
         * The addition in the lower 64 bits
         */
        uint64_t addend = (uint64_t)1 << bits;
        uint64_t old_low = low;
        low += addend;

        /*
         * If 'low' wrapped around (overflowed), propagate a 1-bit carry to high
         */
        if (low < old_low) {
            high += 1U;
        }
    }

    /*
     * Convert back to big-endian and write into place
    high = __builtin_bswap64(high);
    low = __builtin_bswap64(low);
     */
    high = htobe64(high);
    low = htobe64(low);

    memcpy(addr, &high, 8);
    memcpy(addr + 8, &low, 8);
}



/*
 * Non-public helper: computes the last IP in a given CIDR block block size (max_bits)
 * by finding the next boundary block and decrementing by 1.
 */
static void get_last_ip_in_block(const uint8_t *start, unsigned int max_bits, uint8_t *last_ip) {
    if (max_bits == 128U) {
        memset(last_ip, 0xFF, 16);
        return;
    }

    memcpy(last_ip, start, 16);
    add_v6_inplace(last_ip, max_bits);

    /* Safely decrement the 128-bit big-endian integer by 1 */
    for (int i = 15; i >= 0; i--) {
        if (last_ip[i] > 0U) {
            last_ip[i]--;
            break;
        }
        last_ip[i] = 0xFFU;
    }
}

/*
 * Non-public helper: determines the largest possible CIDR block size (prefix)
 * that perfectly fits within the remaining IP range boundary.
 */
static unsigned int find_max_valid_bits(const uint8_t *start, const uint8_t *end_bytes) {
    unsigned int max_bits = count_trailing_zeros_v6(start);
    if (max_bits > 128U) {
        max_bits = 128U;
    }

    /* Shrink prefix size until the block fits completely within end_bytes */
    while (max_bits > 0U) {
        uint8_t last_ip_in_block[16];
        get_last_ip_in_block(start, max_bits, last_ip_in_block);

        if (compare_uint8_t(last_ip_in_block, end_bytes) <= 0) {
            break;
        }
        max_bits--;
    }
    return max_bits;
}

int range_to_cidrs_v6(const uint8_t *start_bytes, const uint8_t *end_bytes, CtCidrs *cidrs) {
    uint8_t start[16] = {};
    CtCidr current_cidr = {};

    memcpy(start, start_bytes, 16);

    while (compare_uint8_t(start, end_bytes) <= 0) {
        unsigned int max_bits = find_max_valid_bits(start, end_bytes);

        /* Build current CIDR struct */
        current_cidr.prefix = (uint8_t)(128U - max_bits);
        current_cidr.addr.family = AF_INET6;
        memcpy(current_cidr.addr.addr.v6.s6_addr, start, 16);

        if (!ct_add_cidr_to_cidrs(cidrs, &current_cidr)) {
            return -1;
        }

        if (max_bits == 128U) {
            break;
        }

        /* Step to the next block */
        add_v6_inplace(start, max_bits);
        if (compare_uint8_t(start, start_bytes) < 0) { // Overflow safety check
            break;
        }
    }
    return 0;
}

#ifdef XXX
static unsigned int count_trailing_zeros_v6(const uint8_t *addr) {
    unsigned int zeros = 0U;

    for (int i = 15U; i >= 0; i--) {
        if (addr[i] == 0U) {
            zeros += 8U;
        } else {
            zeros += (unsigned int)__builtin_ctz(addr[i]);
            break;
        }
    }
    return zeros;
}

static void add_v6_inplace(uint8_t *addr, unsigned int bits) {
    size_t carry = (size_t)1 << (bits % 8U);
    int start_idx = 15 - (int)(bits / 8U);

    for (int i = start_idx; i >= 0; i--) {
        size_t sum = (size_t)addr[i] + carry;
        addr[i] = (uint8_t)(sum & 0xFFU);
        carry = sum >> 8U;
        if (carry == 0U) {
            break;
        }
    }
}

/*
 * Non-public helper
 */
int old_range_to_cidrs_v6(const uint8_t *start_bytes, const uint8_t *end_bytes, CtCidrs *cidrs) {
    uint8_t start[16] = {};
    unsigned int max_bits = 0U;
    CtCidr current_cidr = {};

    memcpy(start, start_bytes, 16);

    while (compare_uint8_t(start, end_bytes) <= 0) {
        max_bits = count_trailing_zeros_v6(start);
        if (max_bits > 128U) {
            max_bits = 128U;
        }

        while (max_bits > 0U) {
            uint8_t test_inc[16];
            uint8_t last_ip_in_block[16];

            memcpy(test_inc, start, 16);
            add_v6_inplace(test_inc, max_bits);
            
            memcpy(last_ip_in_block, start, 16);
            if (max_bits == 128U) {
                memset(last_ip_in_block, 0xFF, 16);
            } else {
                add_v6_inplace(last_ip_in_block, max_bits);
                for (int i = 15; i >= 0; i--) {
                    if (last_ip_in_block[i] > 0U) { 
                        last_ip_in_block[i]--; 
                        break; 
                    } 

                    last_ip_in_block[i] = 0xFFU; 
                }
            }

            if (compare_uint8_t(last_ip_in_block, end_bytes) <= 0) {
                break; 
            }
            max_bits--;
        }

        current_cidr.prefix = (uint8_t)(128U - max_bits);
        current_cidr.addr.family = AF_INET6;
        memcpy(current_cidr.addr.addr.v6.s6_addr, start, 16);

        if (!ct_add_cidr_to_cidrs(cidrs, &current_cidr)) {
            return -1;
        }

        if (max_bits == 128U) {
            break;
        }

        add_v6_inplace(start, max_bits);
        if (compare_uint8_t(start, start_bytes) < 0) {
            break; 
        }
    }
    return 0;
}
#endif
