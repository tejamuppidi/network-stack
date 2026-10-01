#include "ipv4_builder.h"
#include "connection.h"

#include <string.h>
#include <arpa/inet.h>

#include "checksum.h"
#include "ipv4_builder.h"

void ipv4_builder_init(packet_t *pkt)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    memset(ip, 0, sizeof(ipv4_header_t));

    ip->version_ihl =
        (IPV4_VERSION << 4) | 5;

    ip->tos = 0;

    ip->identification = htons(0);

    ip->flags_fragment = htons(0x4000);

    ip->ttl = 64;

    ip->protocol = 0;

    ip->checksum = 0;

    packet_set_length(
        pkt,
        IPV4_HEADER_SIZE);
}

void ipv4_builder_set_source(
    packet_t *pkt,
    uint32_t src_ip)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    ip->src_ip = src_ip;
}

void ipv4_builder_set_destination(
    packet_t *pkt,
    uint32_t dst_ip)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    ip->dst_ip = dst_ip;
}

void ipv4_builder_set_protocol(
    packet_t *pkt,
    uint8_t protocol)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    ip->protocol = protocol;
}

void ipv4_builder_set_ttl(
    packet_t *pkt,
    uint8_t ttl)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    ip->ttl = ttl;
}

void ipv4_builder_finalize(
    packet_t *pkt,
    uint16_t payload_length)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    ip->total_length =
        htons(IPV4_HEADER_SIZE +
              payload_length);

    ip->checksum = 0;

    ip->checksum = htons(
    ipv4_checksum(
        ip,
        IPV4_HEADER_SIZE));
        
    packet_set_length(
        pkt,
        IPV4_HEADER_SIZE +
        payload_length);
}

void tcp_builder_finalize(
    packet_t *pkt,
    const uint8_t *payload,
    uint16_t payload_len)
{
    ipv4_header_t *ip =
        (ipv4_header_t *)packet_buffer(pkt);

    tcp_header_t *tcp =
        (tcp_header_t *)
        (packet_buffer(pkt) + IPV4_HEADER_SIZE);

    uint8_t *payload_dest =
        packet_buffer(pkt) +
        IPV4_HEADER_SIZE +
        TCP_HEADER_SIZE;

    /* Copy payload into packet buffer */
    if (payload != NULL && payload_len > 0)
    {
        memcpy(
            payload_dest,
            payload,
            payload_len);
    }

    /*
     * Finalize IPv4 header.
     * IPv4 payload = TCP header + TCP payload
     */
    ipv4_builder_finalize(
        pkt,
        TCP_HEADER_SIZE + payload_len);

   /* Clear old TCP checksum */
tcp->checksum = 0;

/*
 * TCP segment begins at the TCP header
 * and contains the TCP header + payload.
 */
uint16_t tcp_segment_length =
    TCP_HEADER_SIZE + payload_len;

/* Calculate TCP checksum */
tcp->checksum = htons(
    tcp_checksum(
        ip,
        (const uint8_t *)tcp,
        tcp_segment_length));

    /* Final packet size */
    packet_set_length(
        pkt,
        IPV4_HEADER_SIZE +
        TCP_HEADER_SIZE +
        payload_len);
}
void ipv4_builder_build_reply(
    packet_t *pkt,
    const tcp_connection_t *conn)
{
    ipv4_builder_init(pkt);

    ipv4_builder_set_source(
        pkt,
        conn->dst_ip);

    ipv4_builder_set_destination(
        pkt,
        conn->src_ip);

    ipv4_builder_set_protocol(
        pkt,
        IP_PROTO_TCP);

    ipv4_builder_set_ttl(
        pkt,
        64);
}


