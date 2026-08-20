/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include "version.h"

/**
 * Returns the current version
 *
 * :return: Returns the current version string.
 */
char *ct_version(void) {
	return CIDRTOOLS_VERSION;
}
