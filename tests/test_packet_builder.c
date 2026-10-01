#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "packet.h"
#include "ipv4_builder.h"
#include "tcp_builder.h"
#include "tcp.h"
#include "ipv4.h"

int main(void)
{
    printf("====================================\n");
    printf(" PACKET BUILDER UNIT TESTS\n");
    printf("====================================\n\n");

    packet_t pkt;

    packet_init(&pkt);

    ipv4_builder_init(&pkt);
    ipv4_builder_set_source(&pkt, inet_addr("10.0.0.1"));
    ipv4_builder_set_destination(&pkt, inet_addr("1.1.1.1"));
    ipv4_builder_set_protocol(&pkt, IP_PROTO_TCP);
    ipv4_builder_set_ttl(&pkt, 64);

    tcp_builder_init(&pkt);
    tcp_builder_set_source_port(&pkt, 50000);
    tcp_builder_set_destination_port(&pkt, 80);
    tcp_builder_set_sequence(&pkt, 1000);
    tcp_builder_set_ack(&pkt, 0);
    tcp_builder_set_flags(&pkt, TCP_SYN);
    tcp_builder_set_window(&pkt, 65535);

    tcp_builder_finalize(&pkt, NULL, 0);

    if (packet_length(&pkt) < IPV4_HEADER_SIZE + TCP_HEADER_SIZE) {
        printf("FAIL: packet too short\n");
        return 1;
    }

    printf("Generated packet length: %u bytes\n",
           packet_length(&pkt));

    printf("PASS\n");
    return 0;
}
