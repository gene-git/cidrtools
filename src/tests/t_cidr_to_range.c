#include "cidrtools.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_ip_struct(const char *label, const CtAddress *ip) {
    char buf[INET6_ADDRSTRLEN];
    size_t bufsz = sizeof(buf);

    buf[0] = '\0';
    (void)ct_ip_address_to_str_r(ip, buf, bufsz);
    printf("%s: %s\n", label, buf);
}

int main() {
    CtCidr cidr = {};
    CtAddress first_ip = {};
    CtAddress last_ip = {};
    char *cidr_str = nullptr;

    /*
     * Test 1: IPv4 Subnet (192.168.1.50 /24)
     * - first: 192.168.1.0
     * - last : 192.168.1.255
     */
    cidr_str = "192.168.1.50/24";

    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }

    if (ct_cidr_to_range(&cidr, &first_ip, &last_ip) == 0) {
        printf("--- IPv4 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("Last ", &last_ip);
    }

    /*
     * Test 2: IPv6 Subnet (2001:db8:abcd:1234::55 /56)
     * - first: 2001:db8:abcd:1200::
     * - last : 2001:db8:abcd:12ff:ffff:ffff:ffff:ffff
     */
    memset(&cidr, 0, sizeof(CtCidr));
    cidr_str = "2001:db8:abcd:1234::55/56";

    if (ct_str_to_cidr_block(cidr_str, &cidr) != 0) {
        printf("[FAIL] ct_str_to_cidr_block error\n");
        return 1;
    }

    if (ct_cidr_to_range(&cidr, &first_ip, &last_ip) == 0) {
        printf("\n--- IPv6 Range Test ---\n");
        printf("prefix: %u\n", cidr.prefix);
        print_ip_struct("Input", &cidr.addr);
        print_ip_struct("First", &first_ip);
        print_ip_struct("Last ", &last_ip);
    }

    return 0;
}

