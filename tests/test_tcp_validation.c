#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "packet.h"
#include "ipv4.h"
#include "tcp.h"
#include "checksum.h"

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

    /* Use valid IPv4 addresses for the TCP pseudo-header. */
    ip.src_ip = inet_addr("10.0.0.1");
    ip.dst_ip = inet_addr("10.0.0.2");

    tcp_header_t tcp;
    memset(&tcp, 0, sizeof(tcp));

    tcp.src_port = htons(50000);
    tcp.dst_port = htons(80);
    tcp.seq = htonl(1000);
    tcp.ack = htonl(0);
    tcp.data_offset = (5 << 4);
    tcp.flags = TCP_SYN;
    tcp.window = htons(65535);

    /*
     * Checksum must be calculated with the checksum field
     * set to zero.
     */
    tcp.checksum = 0;

    memcpy(buffer, &tcp, sizeof(tcp));

    tcp.checksum =
       htons(tcp_checksum(
          &ip,
          buffer,
          sizeof(tcp)));

    memcpy(buffer, &tcp, sizeof(tcp));

    printf("Calculated TCP checksum: 0x%04X\n",
           ntohs(tcp.checksum));

    /*
     * Test 1: Valid TCP packet.
     */
    int result =
        tcp_validate(
            &ip,
            buffer,
            sizeof(tcp));

    if (result != 0)
    {
        printf("PASS: valid TCP packet accepted\n");
    }
    else
    {
        printf("FAIL: valid TCP packet rejected\n");
        return 1;
    }

    /*
     * Test 2: Short packet.
     */
    if (tcp_validate(&ip, buffer, 10) == 0)
    {
        printf("PASS: short packet rejected\n");
    }
    else
    {
        printf("FAIL: short packet accepted\n");
        return 1;
    }

    /*
     * Test 3: Corrupted checksum.
     */
    buffer[16] ^= 0xFF;

    if (tcp_validate(&ip, buffer, sizeof(tcp)) == 0)
    {
        printf("PASS: corrupted checksum rejected\n");
    }
    else
    {
        printf("FAIL: corrupted checksum accepted\n");
        return 1;
    }

    printf("\n====================================\n");
    printf(" ALL TCP VALIDATION TESTS PASSED\n");
    printf("====================================\n");

    return 0;
}
