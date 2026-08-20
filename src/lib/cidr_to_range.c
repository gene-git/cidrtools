/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * We have 2 versions of the IPv6 code
 * - 16 x  8 bit / 1 byte blocks
 * -  4 x 32 bit / 8 byte blocks (original, slower)
 *
 * The larger block size is faster.
 */

static int cidr_range_v4(const CtCidr *cidr, CtAddress *first, CtAddress *mid, CtAddress *last) {

    if (cidr->prefix > 32) {
        return -1;
    }

    /*
     * host byte order so bit-shifting simple
     * mask make sure to handle prefix == 0 to avoid wonky shift.
     */
    uint32_t host_ip = ntohl(cidr->addr.addr.v4.s_addr);
    uint32_t mask = (cidr->prefix == 0) ? 0 : (~0U << (uint32_t)(32 - cidr->prefix));

    /*
     * Use mask and inverse mask for first/last ip
     */
    uint32_t first_host = host_ip & mask;
    uint32_t last_host = host_ip | (~mask);
    uint32_t mid_host = first_host + ((last_host - first_host) / 2);

    /*
     * back to network byte order
     */
    first->addr.v4.s_addr = htonl(first_host);
    last->addr.v4.s_addr = htonl(last_host);
    mid->addr.v4.s_addr = htonl(mid_host);

    return 0;
}

/*
 * This does it in 1 byte blocks (max of 16 blocks)
 *
 * Example using /60
 *
 * int full_bytes = 60 / 8;     // Result: 7
 * int remaining_bits = 60 % 8; // Result: 4
 *
 * First 8 bytes (0 through 6) are locked inside the network prefix.
 * Byte 7 (the 8th byte) is the boundary byte, containing 4 network bits and 4 host bits.
 * Bytes 8 through 15 are pure host bits.
 *
 * The first loop over bytes 0 - 6 - no changes are made. 
 * These base address bytes are same for first and last IP:
 *
 * bytes 7 is the boundary byte - some network bits and some host bits (4 and 4 here)
 *
 * Let’s assume the value of input Byte 7 in binary is 10111101 (0xBD).
 *
 * Mask : mask = (0xFFU << (8 - 4)); // Shifts 0xFF left by 4 bits
 * 0xFF = 11111111
 * Shift left by 4 results in => 11110000 (0xF0)
 * Therefore, mask = 11110000
 * inverse mask (~mask) is: 00001111 (0x0F)
 *
 * -----
 * First Address Byte : (&) with the mask - this sets the 4 host bits to 0: 
 * -----
 *
 *   1011 1101  (Original Byte 7: 0xBD)
 * & 1111 0000  (Mask: 0xF0)
 * -----------
 *   1011 0000  (Result: 0xB0)
 *
 * -----
 * Last Address Byte : (|) with inverse mask - sets host bits and keeps network bits
 * -----
 *    1011 1101  (Original Byte 7: 0xBD)
 *  | 0000 1111  (Inverse Mask: 0x0F)
 * -----------
 *    1011 1111  (Result: 0xBF)
 *
 * -----
 * Final Loop when prefix is not divisible by 8 and some left over bytes     
 * -----
 *
 *  These bytes are past the prefix boundary, they are all host bits. 
 *  No complex bitwise math or masking. 
 *  The final loop runs from (in this case) from i = 8 to i < 16:
 *
 *  for (int i = 8; i < 16; i++) {
 *      first->addr.v6.s6_addr[i] = 0x00U;    // Set all trailing host bits to 0
 *      last->addr.v6.s6_addr[i]  = 0xFFU;    // Set all trailing host bits to 1
 *   }
 *
 */

#ifdef v6_8_BIT_BLOCKS
static int cidr_range_v6_8bit(const CtCidr *cidr, CtAddress *first, CtAddress *last) {

    if (cidr->prefix > 128U) {
        return -1;
    }

    /*
     * Determine how many full bytes are in prefix 
     * Then deal with the leftover bits after that
     */
    int full_bytes = cidr->prefix / 8;
    int remaining_bits = cidr->prefix % 8;

    /*
     * Copy the identical 8-bit network blocks
     */
    for (int i = 0; i < full_bytes; i++) {
        uint8_t byte = cidr->addr.addr.v6.s6_addr[i];
        first->addr.v6.s6_addr[i] = byte;
        last->addr.v6.s6_addr[i] = byte;
    }

    /*
     *  Handle the partial boundary 1-byte block (if it exists)
     */
    if (full_bytes < 16) {
        uint8_t byte = cidr->addr.addr.v6.s6_addr[full_bytes];
        uint8_t mask = (remaining_bits == 0) ? 0x00U : (uint8_t)(0xFFU << (8 - remaining_bits));

        first->addr.v6.s6_addr[full_bytes] = byte & mask;
        last->addr.v6.s6_addr[full_bytes] = byte | ~mask;

        /*
         *  Fill the rest of the host bytes with 0s or 1s (0x00 for first, 0xFF for last)
         */
        for (int i = full_bytes + 1; i < 16; i++) {
            first->addr.v6.s6_addr[i] = 0x00U;
            last->addr.v6.s6_addr[i] = 0xFFU;
        }
    }

    return 0;
}
#endif

/*
 * This works using 32 bit blocks. (max of 4 blocks)
 *
 * Example using /60
 *
 * int full_blocks = 60 / 32;       // Result: 1
 * int remaining_bits = 60 % 32;    // Result: 28
 *
 * Block 0 is locked inside the network prefix (32 bits used).
 * Block 1 is the boundary block, containing 28 network bits and 4 host bits (32 + 28 = 60).
 * Blocks 2 and 3 are pure host bits.
 *
 * Full Blocks (1)
 * The first loop runs for i = 0 only. 
 * It copies the first 32-bit chunk from the source to both first and last:
 *  first->addr.v6.s6_addr32[0] = cidr->addr.addr.v6.s6_addr32[0];
 *  last->addr.v6.s6_addr32[0]  = cidr->addr.addr.v6.s6_addr32[0];
 *
 * Boundary Block (Block 1)
 * where full_blocks == 1. 
 * Let's assume the raw network byte order value of s6_addr32[1] is 0x1122334D.
 *
 * Convert to Host Byte Order (ntohl)
 * x86-64 is little endian so reading 0x1122334D directly results in reversed bytes. 
 * ntohl() executes a 1-cycle bswap instruction to put it in native order:
 * (if on big endian then ?)
 * 
 *  uint32_t host_block = ntohl(0x1122334D);    // Result: 0x4D332211
 *
 *   In binary, 0x4D332211 ends with the byte 0x4D, which is 0100 1101.
 *
 * mask must preserve the first 28 bits and clear the final 4 bits
 *  mask = 0xFFFFFFFFU << (32 - 28);    // Shift left by 4
 *  => 0xFFFFFFF0.
 * In binary, the boundary edge looks like this: ... 1111 0000
 *
 * inverse mask (~mask) becomes 0x0000000F (... 0000 1111).
 *
 * -----
 * First
 * -----
 *    0x4D332211  (host_block)
 *  & 0xFFFFFFF0  (mask)
 *  ------------
 *    0x4D332210  (Host result for 'first')
 *
 * -----
 * Last
 * -----
 *
 *   0x4D332211  (host_block)
 * | 0x0000000F  (~mask)
 * ------------
 *   0x4D33221F  (Host result for 'last')
 *
 * Restore network bytes order:
 *
 * first->addr.v6.s6_addr32[1] = htonl(0x4D332210);     // Stores: 0x1022334D
 *  last->addr.v6.s6_addr32[1] = htonl(0x4D33221F);     // Stores: 0x1F22334D
 *
 * All remaining blocks are host bits:
 * The final loop (here) runs from i = 2 to i < 4 
 * and fills them without any conversions needed:
 *
 *  for (int i = 2; i < 4; i++) {
 *      first->addr.v6.s6_addr32[i] = 0x00000000U;      // All host bits to 0
 *       last->addr.v6.s6_addr32[i] = 0xFFFFFFFFU;      // All host bits to 1
 *  }
 *
 */

static int cidr_range_v6(const CtCidr *cidr, CtAddress *first, CtAddress *mid, CtAddress *last) {

    if (cidr->prefix > 128U) {
        return -1;
    }

    /*
     * How many full 32-bit blocks are locked by the prefix
     */
    uint8_t full_blocks = cidr->prefix / 32;
    uint8_t remaining_bits = cidr->prefix % 32;

    /*
     *  Copy the identical 32-bit network blocks
     */
    for (int i = 0; i < full_blocks; i++) {
        uint32_t block = cidr->addr.addr.v6.s6_addr32[i];
        first->addr.v6.s6_addr32[i] = block;
        last->addr.v6.s6_addr32[i] = block;
        mid->addr.v6.s6_addr32[i] = block;
    }

    /*
     *  Handle the partial boundary 32-bit block (if it exists)
     */
    if (full_blocks < 4) {
        /*
         *  Convert block from Network Byte Order to Host Byte Order
         */
        uint32_t host_block = ntohl(cidr->addr.addr.v6.s6_addr32[full_blocks]);

        /*
         * Generate a 32-bit mask (handles 0 cleanly)
         */
        uint32_t mask = (remaining_bits == 0) ? 0x00000000U : (0xFFFFFFFFU << (32U - remaining_bits));

        /*
         * bitwise logic always in Host Byte Order, then convert back to Network Order
         */
        uint32_t first_host = host_block & mask;
        uint32_t last_host = host_block | ~mask;

        first->addr.v6.s6_addr32[full_blocks] = htonl(first_host);
        last->addr.v6.s6_addr32[full_blocks] = htonl(last_host);

        /*
         * Fill the rest of the host blocks (all zeros or all ones)
         */
        for (int i = full_blocks + 1; i < 4; i++) {
            first->addr.v6.s6_addr32[i] = 0x00000000U;
            last->addr.v6.s6_addr32[i]  = 0xFFFFFFFFU;
        }

        /*
         * The mid point - 
         *  - remaining_bits == 0 => mid bit is top of next block
         *  - remaining_bits > 0 => mid bit is inside the current block.
         */
        if (remaining_bits > 0) {
            uint32_t mid_host = first_host;

            /*
             * Add a midpoint bit only if the total subnet has more than 1 host bit.
             * e.g /127 only has 1 host bit so mid == first
             */
            if (cidr->prefix < 127U) {
                mid_host |= (1U << (31U - remaining_bits));
            }
            mid->addr.v6.s6_addr32[full_blocks] = htonl(mid_host);

            /* 
             * All remaining blocks after the mid bit are empty (zeros)
             */
            for (int i = full_blocks + 1; i < 4; i++) {
                mid->addr.v6.s6_addr32[i] = 0x00000000U;
            }
        } else {
            /*
             *  current block is entirely a network block, so it matches first_host 
             */
            mid->addr.v6.s6_addr32[full_blocks] = htonl(first_host);

            /*
             * The next block gets the midpoint high bit dropped into it
             */
            if (full_blocks + 1 < 4) {
                mid->addr.v6.s6_addr32[full_blocks + 1] = htonl(0x80000000U);
            }

            /*
             * Any remaining blocks are cleared to zeros
             */
            for (int i = full_blocks + 2; i < 4; i++) {
                mid->addr.v6.s6_addr32[i] = 0x00000000U;
            }
        }
    }

    return 0;
}



/**
 * Calculates the first and last IP addresses of a cidr block subnet.
 *
 * :param cidr: The cidr block to examine.
 * :param first: The first IP address in the cidr.
 * :param last: The last IP address in the cidr.
 *
 * :returns: 0 on success, or -1 on invalid input.
 */
int ct_cidr_to_range(const CtCidr *cidr, CtAddress *first, CtAddress *last) {

    CtAddress mid = {};

    return ct_cidr_to_range_mid(cidr, first, &mid, last);
}

/**
 * Calculates the first, middle and last IP addresses of a cidr block subnet.
 *
 * :param cidr: The cidr block to examine.
 * :param first: The first IP address in the cidr.
 * :param mid: The middle IP address in the cidr.
 * :param last: The last IP address in the cidr.
 *
 * :returns: 0 on success, or -1 on invalid input.
 */
int ct_cidr_to_range_mid(const CtCidr *cidr, CtAddress *first, CtAddress *mid, CtAddress *last) {

    if (!cidr || !first || !last) {
        return -1;
    }

    /*
     *  Set the family flags on our output structures 
     */
    first->family = cidr->addr.family;
    last->family = cidr->addr.family;
    mid->family = cidr->addr.family;

    switch (cidr->addr.family) {
        case AF_INET:
            return cidr_range_v4(cidr, first, mid, last);

        case AF_INET6:
            return cidr_range_v6(cidr, first, mid, last);
        default:
            return -1;
    }

    return 0;
}
