#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "packet.h"
#include "ipv4.h"
#include "tcp.h"

int main(void)
{
    printf("====================================\n");
    printf(" TCP VALIDATION UNIT TESTS\n");
    printf("====================================\n\n");

    uint8_t buffer[64] = {0};

    ipv4_header_t ip;
    memset(&ip, 0, sizeof(ip));

    ip.version_ihl = (4 << 4) | 5;
    ip.protocol = IP_PROTO_TCP;

    tcp_header_t tcp;
    memset(&tcp, 0, sizeof(tcp));

    tcp.data_offset = (5 << 4);

    memcpy(buffer, &tcp, sizeof(tcp));

    int result = tcp_validate(&ip, buffer, sizeof(tcp));

    if (result != 0) {
        printf("PASS: valid TCP header accepted\n");
    } else {
        printf("FAIL: valid TCP header rejected\n");
        return 1;
    }

    if (tcp_validate(&ip, buffer, 10) == 0) {
        printf("PASS: short packet rejected\n");
    } else {
        printf("FAIL: short packet accepted\n");
        return 1;
    }

    printf("\n====================================\n");
    printf(" TCP VALIDATION TESTS COMPLETE\n");
    printf("====================================\n");

    return 0;
}
