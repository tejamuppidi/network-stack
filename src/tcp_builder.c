#include "tcp_builder.h"
#include "connection.h"
#include "checksum.h"
#include "ipv4_builder.h"

#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

static tcp_header_t *tcp_header(packet_t *pkt)
{
    return (tcp_header_t *)
        (packet_buffer(pkt) + IPV4_HEADER_SIZE);
}

/*
 * Calculate the currently available
 * TCP receive window.
 *
 * The advertised window represents
 * how much additional application
 * data this connection can buffer.
 */
static uint16_t tcp_available_window(
    const tcp_connection_t *conn)
{
    if (conn == NULL)
        return 0;

    if (conn->recv_buffer_len >=
        MAX_TCP_PAYLOAD_BUFFER)
    {
        return 0;
    }

    uint32_t available =
        MAX_TCP_PAYLOAD_BUFFER -
        conn->recv_buffer_len;

    /*
     * TCP window field is 16 bits.
     */
    if (available > UINT16_MAX)
        available = UINT16_MAX;

    return (uint16_t)available;
}

void tcp_builder_init(packet_t *pkt)
{
    tcp_header_t *tcp = tcp_header(pkt);

    memset(tcp, 0, sizeof(tcp_header_t));

    /* TCP Header Length = 20 bytes */
    tcp->data_offset = (5 << 4);

    tcp->window = htons(65535);

    packet_set_length(
        pkt,
        IPV4_HEADER_SIZE +
        TCP_HEADER_SIZE);
}

void tcp_builder_set_source_port(
        packet_t *pkt,
        uint16_t port)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->src_port = htons(port);
}

void tcp_builder_set_destination_port(
        packet_t *pkt,
        uint16_t port)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->dst_port = htons(port);
}

void tcp_builder_set_sequence(
        packet_t *pkt,
        uint32_t seq)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->seq = htonl(seq);
}

void tcp_builder_set_ack(
        packet_t *pkt,
        uint32_t ack)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->ack = htonl(ack);
}

void tcp_builder_set_flags(
        packet_t *pkt,
        uint8_t flags)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->flags = flags;
}

void tcp_builder_set_window(
        packet_t *pkt,
        uint16_t window)
{
    tcp_header_t *tcp = tcp_header(pkt);

    tcp->window = htons(window);
    printf(
        "[TCP WINDOW] Advertising window = %u bytes\n",
        window);
}

/*
 * Build a SYN-ACK TCP header
 */
void tcp_builder_build_syn_ack(
    packet_t *pkt,
    const tcp_connection_t *conn)
{
    tcp_builder_init(pkt);

    /* Server -> Client */
    tcp_builder_set_source_port(
        pkt,
        conn->dst_port);

    tcp_builder_set_destination_port(
        pkt,
        conn->src_port);

    /* Sequence Numbers */
    tcp_builder_set_sequence(
        pkt,
        conn->server_seq);

    tcp_builder_set_ack(
        pkt,
        conn->recv_next);

    /* SYN + ACK */
    tcp_builder_set_flags(
        pkt,
        TCP_SYN | TCP_ACK);

    tcp_builder_set_window(
    pkt,
    tcp_available_window(conn));
}

/*
 * Build a pure ACK TCP header
 */
void tcp_builder_build_ack(
    packet_t *pkt,
    const tcp_connection_t *conn)
{
    tcp_builder_init(pkt);

    /* Server -> Client */
    tcp_builder_set_source_port(
        pkt,
        conn->dst_port);

    tcp_builder_set_destination_port(
        pkt,
        conn->src_port);

    /* Current sequence numbers */
    tcp_builder_set_sequence(
        pkt,
        conn->send_next);

    tcp_builder_set_ack(
        pkt,
        conn->recv_next);

    /* ACK only */
    tcp_builder_set_flags(
        pkt,
        TCP_ACK);

    tcp_builder_set_window(
    pkt,
    tcp_available_window(conn));
}

/*
 * Build a TCP PSH + ACK packet.
 */
void tcp_builder_build_data(
    packet_t *pkt,
    const tcp_connection_t *conn)
{
    tcp_builder_init(pkt);

    /* Server -> Client */
    tcp_builder_set_source_port(
        pkt,
        conn->dst_port);

    tcp_builder_set_destination_port(
        pkt,
        conn->src_port);

    /*
     * Server sequence number.
     */
    tcp_builder_set_sequence(
        pkt,
        conn->send_next);

    /*
     * Acknowledge all received client data.
     */
    tcp_builder_set_ack(
        pkt,
        conn->recv_next);

    /*
     * Application data.
     *
     * PSH -> deliver data to application
     * ACK -> acknowledge client data
     */
    tcp_builder_set_flags(
        pkt,
        TCP_PSH | TCP_ACK);

    tcp_builder_set_window(
        pkt,
        65535);
}

/*

* Build a TCP FIN + ACK packet.
*
* Used when the server closes
* its side of the TCP connection.
  */
  void tcp_builder_build_fin_ack(
  packet_t *pkt,
  const tcp_connection_t *conn)
  {
  tcp_builder_init(pkt);

  /* Server -> Client */
  tcp_builder_set_source_port(
  pkt,
  conn->dst_port);

  tcp_builder_set_destination_port(
  pkt,
  conn->src_port);

  /*

  * Current server sequence number.
    */
    tcp_builder_set_sequence(
    pkt,
    conn->send_next);

  /*

  * Acknowledge everything received
  * from the client, including FIN.
    */
    tcp_builder_set_ack(
    pkt,
    conn->recv_next);

  /*

  * FIN + ACK
    */
    tcp_builder_set_flags(
    pkt,
    TCP_FIN | TCP_ACK);

  tcp_builder_set_window(
    pkt,
    tcp_available_window(conn));
  }

