#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

#define IPV4_VERSION 4

#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

#pragma pack(push,1)

typedef struct
{
    uint8_t version_ihl;

    uint8_t tos;

    uint16_t total_length;

    uint16_t identification;

    uint16_t flags_fragment;

    uint8_t ttl;

    uint8_t protocol;

    uint16_t checksum;

    uint32_t src_ip;

    uint32_t dst_ip;

} ipv4_header_t;

#pragma pack(pop)

#define IPV4_HEADER_SIZE sizeof(ipv4_header_t)

int ipv4_header_length(const ipv4_header_t *ip);

int ipv4_version(const ipv4_header_t *ip);

void ipv4_print(const ipv4_header_t *ip);

/*
 * Validate an IPv4 packet.
 *
 * Returns:
 *   1 -> valid
 *   0 -> invalid
 */
int ipv4_validate(
    const uint8_t *packet,
    uint16_t packet_len
);

/*
 * Verify IPv4 header checksum.
 *
 * Returns:
 *   1 -> valid
 *   0 -> invalid
 */
int ipv4_checksum_valid(
    const ipv4_header_t *ip,
    uint16_t header_len
);

#endif
