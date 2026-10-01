#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ipv4.h"

int main(void)
{
    printf("====================================\n");
    printf(" IPv4 VALIDATION UNIT TESTS\n");
    printf("====================================\n\n");

    uint8_t buffer[64] = {0};

    ipv4_header_t *ip = (ipv4_header_t *)buffer;

    ip->version_ihl = (4 << 4) | 5;
    ip->total_length = 0;
    ip->protocol = IP_PROTO_TCP;

    printf("Test 1: Packet shorter than IPv4 header...\n");

    if (ipv4_validate(buffer, 10) == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        return 1;
    }

    printf("\nTest 2: Invalid IPv4 version...\n");

    ip->version_ihl = (6 << 4) | 5;

    if (ipv4_validate(buffer, sizeof(ipv4_header_t)) == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        return 1;
    }

    printf("\n====================================\n");
    printf(" IPv4 VALIDATION TESTS COMPLETE\n");
    printf("====================================\n");

    return 0;
}
