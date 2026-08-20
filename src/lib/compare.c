/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <stdint.h>
#include <string.h>

/*
 * Safe 128-bit comparison helper for IPv6 addresses.
 * Optimized via compiler inlining of 16-byte blocks.
 */
int compare_uint8_t(const uint8_t *a, const uint8_t *b) {
    return memcmp(a, b, 16);
}

/*
 * Explicit qsort-compliant wrapper to prevent casting issues.
 */
int compare_uint8_t_qsort(const void *a, const void *b) {
    return memcmp(*(const uint8_t **)a, *(const uint8_t **)b, 16);
}

