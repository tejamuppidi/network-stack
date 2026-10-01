#include "checksum.h"

#include <string.h>
#include <arpa/inet.h>

uint16_t checksum(const void *data, size_t length)
{
    const uint8_t *bytes = data;

    uint32_t sum = 0;

    while (length > 1)
    {
        sum += (bytes[0] << 8) | bytes[1];

        bytes += 2;
        length -= 2;
    }

    if (length)
    {
        sum += bytes[0] << 8;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

uint16_t ipv4_checksum(const void *header, size_t length)
{
    return checksum(header, length);
}

/* ---------- TCP checksum ---------- */

#pragma pack(push,1)

typedef struct
{
    uint32_t src_ip;
    uint32_t dst_ip;

    uint8_t zero;

    uint8_t protocol;

    uint16_t tcp_length;

} tcp_pseudo_header_t;

#pragma pack(pop)

uint16_t tcp_checksum(
    const ipv4_header_t *ip,
    const uint8_t *tcp_segment,
    uint16_t tcp_segment_length)
{
    if (ip == NULL ||
        tcp_segment == NULL ||
        tcp_segment_length == 0)
    {
        return 0;
    }

    tcp_pseudo_header_t pseudo;

    memset(
        &pseudo,
        0,
        sizeof(pseudo));

    pseudo.src_ip = ip->src_ip;
    pseudo.dst_ip = ip->dst_ip;

    pseudo.zero = 0;
    pseudo.protocol = IP_PROTO_TCP;

    pseudo.tcp_length =
        htons(tcp_segment_length);


    /*
     * Pseudo header + complete TCP segment.
     */
    size_t required =
        sizeof(pseudo) +
        tcp_segment_length;


    if (required > 4096)
    {
        return 0;
    }


    uint8_t buffer[4096];

    size_t offset = 0;


    memcpy(
        buffer + offset,
        &pseudo,
        sizeof(pseudo));

    offset += sizeof(pseudo);


    memcpy(
        buffer + offset,
        tcp_segment,
        tcp_segment_length);

    offset += tcp_segment_length;


    return checksum(
        buffer,
        offset);
}
