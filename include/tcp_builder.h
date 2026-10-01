#ifndef TCP_BUILDER_H
#define TCP_BUILDER_H

#include <stdint.h>

#include "packet.h"
#include "tcp.h"
#include "ipv4.h"
#include "connection.h"

/* Initialize a TCP header immediately after the IPv4 header */
void tcp_builder_init(packet_t *pkt);

/* Source port */
void tcp_builder_set_source_port(
    packet_t *pkt,
    uint16_t port);

/* Destination port */
void tcp_builder_set_destination_port(
    packet_t *pkt,
    uint16_t port);

/* Sequence number */
void tcp_builder_set_sequence(
    packet_t *pkt,
    uint32_t seq);

/* Acknowledgement number */
void tcp_builder_set_ack(
    packet_t *pkt,
    uint32_t ack);

/* TCP flags */
void tcp_builder_set_flags(
    packet_t *pkt,
    uint8_t flags);

/* Window size */
void tcp_builder_set_window(
    packet_t *pkt,
    uint16_t window);
    
/*
 * Finalize TCP packet.
 *
 * payload      -> pointer to TCP payload
 * payload_len  -> payload size
 *
 * Computes:
 *      TCP checksum
 *      IPv4 total length
 */
void tcp_builder_finalize(
    packet_t *pkt,
    const uint8_t *payload,
    uint16_t payload_len);
    
void tcp_builder_build_syn_ack(
    packet_t *pkt,
    const tcp_connection_t *conn);
    
void tcp_builder_build_ack(
    packet_t *pkt,
    const tcp_connection_t *conn);
    
/*
 * Build a TCP data packet.
 *
 * The payload itself is attached by
 * tcp_builder_finalize().
 */
void tcp_builder_build_data(
    packet_t *pkt,
    const tcp_connection_t *conn);
    
void tcp_builder_build_fin_ack(
    packet_t *pkt,
    const tcp_connection_t *conn);

#endif
