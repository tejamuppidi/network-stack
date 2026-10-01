#ifndef TCP_H
#define TCP_H

#include <stdint.h>
#include "ipv4.h"

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_ECE 0x40
#define TCP_CWR 0x80

#pragma pack(push,1)

typedef struct
{
    uint16_t src_port;

    uint16_t dst_port;

    uint32_t seq;

    uint32_t ack;

    uint8_t data_offset;

    uint8_t flags;

    uint16_t window;

    uint16_t checksum;

    uint16_t urgent;

} tcp_header_t;

#pragma pack(pop)

#define TCP_HEADER_SIZE sizeof(tcp_header_t)

int tcp_header_length(const tcp_header_t *tcp);

int tcp_is_fin(const tcp_header_t *tcp);

void tcp_print(const tcp_header_t *tcp);

void tcp_process(
    int tun_fd,
    ipv4_header_t *ip,
    tcp_header_t *tcp);

/*
 * Validate a TCP segment.
 *
 * tcp_packet points to the start of the TCP header.
 * tcp_packet_len is the TCP segment length,
 * including header and payload.
 *
 * Returns:
 *   1 -> valid
 *   0 -> invalid
 */
int tcp_validate(
    const ipv4_header_t *ip,
    const uint8_t *tcp_packet,
    uint16_t tcp_packet_len
);

/*
 * Validate TCP checksum.
 *
 * Returns:
 *   1 -> valid
 *   0 -> invalid
 */
int tcp_checksum_valid(
    const ipv4_header_t *ip,
    const uint8_t *tcp_segment,
    uint16_t tcp_segment_length
);

#endif
