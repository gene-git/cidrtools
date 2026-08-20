/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */

#include "cidrtools.h"
#include <stdio.h>


int main() {

    char *vers = ct_version();
    if (vers) {
        printf("Test 1 -> version: %s\n", vers);
        return 0;
    }         return 1;
   
}

