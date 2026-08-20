// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *  cidrtools-export.h
 *  Build with -fvisibility=hidden
 */
#ifndef CIDRTOOLS_EXPORT_H
#define CIDRTOOLS_EXPORT_H

#if defined(__GNUC__) || defined(__clang__)
#define CIDRTOOLS_EXPORT __attribute__((visibility("default")))
#else
#define CIDRTOOLS_EXPORT
#endif

#endif
