#ifndef IPV4_BUILDER_H
#define IPV4_BUILDER_H

#include <stdint.h>

#include "packet.h"
#include "ipv4.h"
#include "connection.h"

/*
 * Initialize an IPv4 header inside the packet buffer.
 */
void ipv4_builder_init(packet_t *pkt);

/*
 * Set source IPv4 address.
 * Pass the address in network byte order
 * (for example: inet_addr("10.0.0.1")).
 */
void ipv4_builder_set_source(
    packet_t *pkt,
    uint32_t src_ip);

/*
 * Set destination IPv4 address.
 */
void ipv4_builder_set_destination(
    packet_t *pkt,
    uint32_t dst_ip);

/*
 * Set transport protocol.
 * Example:
 *      IP_PROTO_TCP
 *      IP_PROTO_UDP
 *      IP_PROTO_ICMP
 */
void ipv4_builder_set_protocol(
    packet_t *pkt,
    uint8_t protocol);

/*
 * Set TTL.
 */
void ipv4_builder_set_ttl(
    packet_t *pkt,
    uint8_t ttl);

/*
 * Finalize IPv4 header.
 *
 * payload_length =
 *      bytes after IPv4 header
 *
 * Calculates:
 *      total length
 *      checksum
 */
void ipv4_builder_finalize(
    packet_t *pkt,
    uint16_t payload_length);

void ipv4_builder_build_reply(
    packet_t *pkt,
    const tcp_connection_t *conn);
    
#endif
