/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
 */
#include "cidrtools.h"
#include <arpa/nameser.h>
#include <bits/getopt_core.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * constants
 */
enum { 
    DEFAULT_COUNT = 24U,
    MAX_COUNT = 256U,
};

/*
 * usage 
 */
static void print_usage(const char *prog_name) {

    (void)fprintf(stderr, "Usage: %s [-n num_to_print] <cidr_string>\n", prog_name);
    (void)fprintf(stderr, "Options:\n");
    (void)fprintf(stderr, "  -n NUM    Number of IPs to print (Default: %u, Max: %u)\n",
            DEFAULT_COUNT, MAX_COUNT);
    (void)fprintf(stderr, "Example:\n");
    (void)fprintf(stderr, "  %s -n 10 64.233.180.0/24\n", prog_name);
    (void)fprintf(stderr, "  %s -n 10 2607:f8b0:4004:c27::8b/64\n", prog_name);
}

/*
 * Parse command-line 
 */
static int parse_command_line(int argc, char *argv[], size_t *count_p, const char **cidr_p) {

    size_t count = DEFAULT_COUNT;
    int opt = 0;
    char *endptr = nullptr;

    while ((opt = getopt(argc, argv, "n:h")) != -1) {       // NOLINT(concurrency-mt-unsafe)
        switch (opt) {
            case 'n': {
                long input_num = strtol(optarg, &endptr, 10);
                if (input_num <= 0) {
                    (void)fprintf(stderr, "Error: Print count must be a positive integer.\n");
                    return 1;
                }

                /*
                 * Cap the number to be printed
                 */
                count = (size_t)input_num;
                if (count > MAX_COUNT) {
                    count = MAX_COUNT;
                }
                break;
            }

            case 'h':
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /*
     *  Ensure we have exactly one positional argument left (the CIDR string)
     *  - We could accept a list but not super useful.
     *  - We could also just accept the first one and ignore the remaining ones.
     */
    if (optind >= argc) {
        (void)fprintf(stderr, "Error: Missing required CIDR string argument.\n");
        print_usage(argv[0]);
        return 1;
    }
    const char *cidr_str = argv[optind];
    *count_p = count;
    *cidr_p = cidr_str;
    return 0;
}

static void output_host(bool ipv6, CtAddress *ip_address) {

    char hostname[NS_MAXDNAME] = {};
    char *ip_str = ct_ip_address_to_str(ip_address);

    if (ip_str) {
        ct_ip_str_to_hostname(ip_str, hostname);

        if (ipv6) {
            printf(" %-44s", ip_str);
        } else {
            printf(" %-19s", ip_str);
        }
        free(ip_str);

        printf("  \t%s\n", hostname);

    } else {
        printf("  \t *error*\n");
    }
}


/**
 * The hostcheck application
 * - given a cidr print a sample of IP addresses in the cidr block 
 *   along with their hostnames via PTR lookup.
 */
int main(int argc, char *argv[]) {
    size_t count = DEFAULT_COUNT;
    const char *cidr_str = nullptr;
    char num_ips_buf[256] = {};

    (void)setlocale(LC_NUMERIC, "");        // NOLINT(concurrency-mt-unsafe)

    if (parse_command_line(argc, argv, &count, &cidr_str) != 0) {
        return 1;
    }

    /* 
     * Parse CIDR string into CtCidr = (CtAddress + prefix)
     */
    CtCidr cidr = {};
    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        (void)fprintf(stderr, "Error: Invalid or malformed CIDR string format '%s'.\n", cidr_str);
        return 1;
    }

    /*
     * Get the :
     * - total number of IPs 
     * - first/last IP address.
     */
    size_t total_num_ips = ct_num_ips(&cidr);
    CtAddress first_ip = {};
    CtAddress last_ip = {};;
    
    /*
     * Assuming your refactored method signature matching the struct update
     */
    if (ct_cidr_to_range(&cidr, &first_ip, &last_ip) != 0) {
        (void)fprintf(stderr, "Error: Failed to locate the iP range for %s.\n", cidr_str);
        return 1;
    }

    /*
     * Print summary of metadata
     * - %' is non-portable but fine on linux
     */
    char *net_str = ct_cidr_to_str(&cidr);
    char *first_str = ct_ip_address_to_str(&first_ip);
    char *last_str = ct_ip_address_to_str(&last_ip);

    if (total_num_ips == SIZE_MAX) {
        (void)snprintf(num_ips_buf, sizeof(num_ips_buf), "> 2^64");
    } else {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat"
        (void)snprintf(num_ips_buf, sizeof(num_ips_buf), "%'zu", total_num_ips);
        #pragma GCC diagnostic pop
    }

    printf("#\n");
    printf("#           Host Check\n");
    printf("# Cidr    : %s\n", net_str ? net_str : "?");
    printf("# First   : %s\n", first_str ? first_str : "?");
    printf("# Last    : %s\n", last_str ? last_str : "?");
    printf("# Num Ips : %s\n", num_ips_buf);

    free(net_str);
    free(first_str);
    free(last_str);

    /*
     * Iterate and print hosts every 'inc' count
     * - print first
     * - ...
     * - print last
     */
    size_t inc = 1U;
    if (total_num_ips <= count) {
        count = total_num_ips;
        inc = 1U;

    } else if (count > 2) {
        inc = (total_num_ips / count);
        count -= 2;
        if (inc == 0) {
            inc = 1U;
        }
    }
    printf("#\n");
    bool ipv6 = ct_is_ipv6(&cidr);

    /*
     * First IP
     */
    output_host(ipv6, &first_ip);
    CtAddress ip_current = first_ip;
    size_t count_ips = 1;

    CtAddress ip_next = {};

    for (size_t i = 0; i < count; i++) {
        /*
         * Stop printing if we exceed the block capacity (crucial for small subnets like /30 or /32)
         */
        if (total_num_ips != SIZE_MAX && count_ips >= total_num_ips) {
            break;
        }

        if (ct_ip_address_increment(&ip_current, inc, &ip_next) != 0) {
            (void)fprintf(stderr, "Warning: Incrementation overflow encountered mid-loop.\n");
            break;
        }
        output_host(ipv6, &ip_next);
        ip_current = ip_next;
        count_ips += inc;
    }
    
    /*
     * Last IP
     */
    if (count_ips < total_num_ips) {
        output_host(ipv6, &last_ip);
    }

    return 0;
}

