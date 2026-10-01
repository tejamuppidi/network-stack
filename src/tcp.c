#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "packet.h"
#include "ipv4_builder.h"
#include "tcp_builder.h"
#include "connection.h"
#include "tcp.h"
#include "http.h"
#include "checksum.h"
#include "tcp_sequence.h"
#include "retransmission.h"

/*
 * Check whether a complete HTTP header
 * has been received.
 *
 * HTTP headers end with:
 *
 * \r\n\r\n
 */
static int http_request_complete(
    const uint8_t *buffer,
    uint32_t length)
{
    if (buffer == NULL || length < 4)
        return 0;

    for (uint32_t i = 0;
         i <= length - 4;
         i++)
    {
        if (buffer[i]     == '\r' &&
            buffer[i + 1] == '\n' &&
            buffer[i + 2] == '\r' &&
            buffer[i + 3] == '\n')
        {
            return 1;
        }
    }

    return 0;
}

/*
 * =====================================
 * Phase 8B:
 * Send ACK with the current expected
 * receive sequence number.
 * =====================================
 */
static void tcp_send_current_ack(
    int tun_fd,
    tcp_connection_t *conn)
{
    packet_t ack_reply;

    packet_init(&ack_reply);

    ipv4_builder_build_reply(
        &ack_reply,
        conn);

    tcp_builder_build_ack(
        &ack_reply,
        conn);

    tcp_builder_finalize(
        &ack_reply,
        NULL,
        0);

    ssize_t written =
        write(
            tun_fd,
            packet_buffer(&ack_reply),
            packet_length(&ack_reply));

    if (written < 0)
    {
        perror("write");
        return;
    }

    printf(
        ">>> ACK Sent (ACK = %u)\n",
        conn->recv_next);
}

static uint32_t tcp_available_send_window(
    tcp_connection_t *conn);

static void tcp_handle_established(
int tun_fd,
tcp_connection_t *conn,
ipv4_header_t *ip,
tcp_header_t *tcp)
{
uint16_t total_length =
ntohs(ip->total_length);


int ip_len =
    ipv4_header_length(ip);

int tcp_len =
    tcp_header_length(tcp);

int payload_length =
    total_length - ip_len - tcp_len;

/*
 * =====================================
 * Phase 8B: Duplicate Packet Handling
 * =====================================
 */

uint32_t received_seq =
    ntohl(tcp->seq);

/*
 * =====================================
 * Phase 9C: ACK Processing
 * =====================================
 *
 * An ACK may arrive with or without
 * application payload.
 */
if (tcp->flags & TCP_ACK)
{
    uint32_t received_ack =
        ntohl(tcp->ack);

    printf(
        ">>> TCP ACK received: %u\n",
        received_ack
    );

    /*
     * Remove packets that have been
     * acknowledged.
     */
    retransmit_ack(
    &g_retransmit_queue,

    conn->src_ip,
    conn->dst_ip,

    conn->src_port,
    conn->dst_port,

    received_ack
);
}


/*
 * A new segment must start exactly at
 * the next sequence number we expect.
 */
if (received_seq != conn->recv_next)
{
    /*
     * Old sequence number:
     * packet was already received.
     */
    if (tcp_seq_lt(
            received_seq,
            conn->recv_next))
    {
        printf(
            ">>> Duplicate TCP segment detected\n");

        printf(
            ">>> Received SEQ = %u\n",
            received_seq);

        printf(
            ">>> Expected SEQ = %u\n",
            conn->recv_next);

        printf(
            ">>> Payload ignored\n");

        /*
         * Re-send ACK so the client knows
         * what data we expect next.
         */
        tcp_send_current_ack(
            tun_fd,
            conn);

        return;
    }


    /*
     * Future sequence number:
     * missing data arrived before this one.
     *
     * This mini stack does not yet have
     * an out-of-order receive buffer.
     */
    if (tcp_seq_gt(
            received_seq,
            conn->recv_next))
    {
        printf(
            ">>> Out-of-order TCP segment\n");

        printf(
            ">>> Received SEQ = %u\n",
            received_seq);

        printf(
            ">>> Expected SEQ = %u\n",
            conn->recv_next);

        printf(
            ">>> Segment dropped\n");

        tcp_send_current_ack(
            tun_fd,
            conn);

        return;
    }
}


/*
 * =====================================
 * Phase 7A: FIN Handling
 * =====================================
 *
 * FIN consumes one sequence number.
 *
 * Therefore:
 *
 * recv_next = client_seq + payload_length + 1
 */

if (tcp_is_fin(tcp))
{
printf("\n");
printf(">>> FIN Received\n");
printf(">>> Client wants to close connection\n");


/*
 * FIN consumes one sequence number.
 *
 * If the packet also contains data,
 * acknowledge both the data and FIN.
 */
conn->recv_next =
    ntohl(tcp->seq) +
    (payload_length > 0 ? payload_length : 0) +
    1;

printf(
    ">>> Recv Next = %u\n",
    conn->recv_next);


/*
 * =====================================
 * Phase 7B: Send ACK for FIN
 * =====================================
 */

packet_t ack_reply;

packet_init(&ack_reply);


/*
 * Build IPv4 reply header.
 */
ipv4_builder_build_reply(
    &ack_reply,
    conn);


/*
 * Build pure TCP ACK.
 *
 * SEQ = conn->send_next
 * ACK = conn->recv_next
 */
tcp_builder_build_ack(
    &ack_reply,
    conn);


/*
 * Finalize packet and calculate
 * checksums.
 *
 * No payload for a pure ACK.
 */
tcp_builder_finalize(
    &ack_reply,
    NULL,
    0);


printf(
    ">>> Sending ACK for FIN (%u bytes)\n",
    packet_length(&ack_reply));


ssize_t written =
    write(
        tun_fd,
        packet_buffer(&ack_reply),
        packet_length(&ack_reply));


if (written < 0)
{
    perror("write");
    return;
}


printf(
    ">>> FIN ACK Sent\n");


/*
 * Client FIN has been acknowledged.
 *
 * We are now waiting for the
 * server application to close
 * its side of the connection.
 */
conn->state = TCP_CLOSE_WAIT;


printf(
    ">>> TCP State changed: CLOSE_WAIT\n");


/*

* =====================================
* Phase 7C: Send Server FIN
* =====================================
  */

packet_t fin_reply;

packet_init(&fin_reply);

/*

* Build IPv4 reply header.
  */
  ipv4_builder_build_reply(
  &fin_reply,
  conn);

/*

* Build TCP FIN + ACK.
*
* SEQ = conn->send_next
* ACK = conn->recv_next
  */
  tcp_builder_build_fin_ack(
  &fin_reply,
  conn);

/*

* Finalize packet.
*
* No application payload.
  */
  tcp_builder_finalize(
  &fin_reply,
  NULL,
  0);

printf(
">>> Sending Server FIN (%u bytes)\n",
packet_length(&fin_reply));

ssize_t fin_written =
write(
tun_fd,
packet_buffer(&fin_reply),
packet_length(&fin_reply));

if (fin_written < 0)
{
perror("write");
return;
}

/*

* FIN consumes one sequence number.
  */
  conn->send_next += 1;

printf(
">>> Server FIN Sent\n");

printf(
">>> Send Next = %u\n",
conn->send_next);

/*

* Wait for the client's final ACK.
  */
  conn->state = TCP_LAST_ACK;

printf(
">>> TCP State changed: LAST_ACK\n");

/*

* Phase 7D:
*
* Wait for final ACK.
  */

return;


}



/*
 * No FIN and no payload.
 */
if (payload_length <= 0)
{
    printf(">>> No Payload\n");
    return;
}


/*
 * =====================================
 * Improved TCP Payload Handling
 * =====================================
 *
 * TCP is a byte stream.
 *
 * Application data may arrive across
 * multiple TCP segments, so buffer the
 * data until a complete HTTP request
 * is available.
 */

uint8_t *payload =
    (uint8_t *)tcp + tcp_len;

printf(
    "\n===== TCP Payload (%d bytes) =====\n",
    payload_length);

fwrite(
    payload,
    1,
    payload_length,
    stdout);

printf(
    "\n==================================\n");


/*
 * Make sure the connection receive
 * buffer has enough space.
 */
if (conn->recv_buffer_len +
        (uint32_t)payload_length >
    MAX_TCP_PAYLOAD_BUFFER)
{
    printf(
        ">>> TCP receive buffer overflow\n");

    /*
     * Do not consume data that we
     * cannot safely store.
     */
    tcp_send_current_ack(
        tun_fd,
        conn);

    return;
}


/*
 * Append this TCP payload to the
 * connection's receive buffer.
 */
memcpy(
    conn->recv_buffer +
        conn->recv_buffer_len,

    payload,

    payload_length);


/*
 * Update buffered length.
 */
conn->recv_buffer_len +=
    (uint32_t)payload_length;


/*
 * Consume the TCP bytes.
 */
conn->recv_next +=
    (uint32_t)payload_length;


printf(
    ">>> Buffered TCP data: %u bytes\n",
    conn->recv_buffer_len);


/*
 * Check whether the complete HTTP
 * header has arrived.
 *
 * HTTP headers end with \r\n\r\n.
 */
if (!http_request_complete(
        conn->recv_buffer,
        conn->recv_buffer_len))
{
    printf(
        ">>> HTTP request incomplete\n");

    printf(
        ">>> Waiting for more TCP data\n");


    /*
     * Acknowledge the data we have
     * successfully buffered.
     */
    tcp_send_current_ack(
        tun_fd,
        conn);

    return;
}


/*
 * =====================================
 * Complete HTTP Request Available
 * =====================================
 */

http_request_t request;

if (parse_http_request(
        conn->recv_buffer,
        conn->recv_buffer_len,
        &request) != 0)
{
    printf(
        ">>> Invalid HTTP request\n");


    /*
     * The TCP data was already accepted
     * into our receive buffer, so ACK it.
     */
    tcp_send_current_ack(
        tun_fd,
        conn);


    /*
     * Clear invalid buffered data.
     */
    conn->recv_buffer_len = 0;

    return;
}


printf("\n===== HTTP Request =====\n");

printf(
    "Method : %s\n",
    request.method);

printf(
    "Path   : %s\n",
    request.path);

printf("========================\n");


/*
 * Build HTTP response.
 */
char http_response[1024];

int http_length =
    build_http_response(
        http_response,
        sizeof(http_response));

if (http_length < 0)
{
    printf(
        ">>> Failed to build HTTP response\n");

    return;
}


printf(
    ">>> HTTP Response built (%d bytes)\n",
    http_length);

/*
 * =====================================
 * TCP Send Window Check
 * =====================================
 *
 * The peer advertises how many bytes
 * it can currently receive.
 *
 * Do not send more unacknowledged data
 * than the available peer window.
 */
uint32_t available_window =
    tcp_available_send_window(conn);

printf(
    ">>> Peer Window      = %u bytes\n",
    conn->peer_window);

printf(
    ">>> Available Window = %u bytes\n",
    available_window);

if ((uint32_t)http_length >
    available_window)
{
    printf(
        ">>> HTTP response exceeds "
        "available TCP window\n");

    printf(
        ">>> Response = %d bytes, "
        "Available = %u bytes\n",
        http_length,
        available_window);

    /*
     * We do not yet implement TCP
     * response segmentation here.
     *
     * Keep the request buffered so
     * it can be processed later when
     * the peer advertises more window.
     */
    return;
}


/*
 * Build reply packet.
 */
packet_t reply;

packet_init(&reply);

ipv4_builder_build_reply(
    &reply,
    conn);


/*
 * Build TCP PSH + ACK header.
 */
tcp_builder_build_data(
    &reply,
    conn);


/*
 * Attach HTTP response and calculate
 * checksums.
 */
tcp_builder_finalize(
    &reply,
    (const uint8_t *)http_response,
    http_length);


printf(
    ">>> Sending HTTP response (%u bytes)\n",
    packet_length(&reply));


/*
 * Remember the first sequence number
 * used by this TCP data packet.
 */
uint32_t seq_start =
    conn->send_next;


/*
 * Send the HTTP response.
 */
ssize_t written =
    write(
        tun_fd,
        packet_buffer(&reply),
        packet_length(&reply));


if (written < 0)
{
    perror("write");
    return;
}


/*
 * The HTTP payload occupies this TCP
 * sequence number range:
 *
 * [seq_start, seq_start + http_length)
 */
uint32_t seq_end =
    seq_start + http_length;


/*
 * Phase 9:
 * Store the successfully transmitted
 * packet until it is acknowledged.
 */
if (retransmit_store(
        &g_retransmit_queue,

        conn->src_ip,
        conn->dst_ip,

        conn->src_port,
        conn->dst_port,

        packet_buffer(&reply),
        packet_length(&reply),

        seq_start,
        seq_end
    ) != 0)
{
    printf(
        "[RETRANSMIT] Failed to store "
        "HTTP response packet\n"
    );
}


/*
 * Advance server sequence number.
 */
conn->send_next = seq_end;


/*
 * The current implementation sends
 * one HTTP response per connection.
 *
 * Clear the buffered request after
 * processing it.
 */
conn->recv_buffer_len = 0;


printf(
    ">>> HTTP Response Sent\n");

printf(
    ">>> Send Next = %u\n",
    conn->send_next);

}

static void tcp_handle_syn_received(
    tcp_connection_t *conn,
    const tcp_header_t *tcp)
{


    if ((tcp->flags & TCP_ACK) &&
    ntohl(tcp->ack) == conn->server_seq + 1)
    {
        printf(">>> ACK Received\n");

        conn->state = TCP_ESTABLISHED;
        conn->send_next = conn->server_seq + 1;
        conn->recv_next = ntohl(tcp->seq);

        printf(">>> Connection Established\n");
    }
}

/*
 * =====================================
 * Phase 7D + 7E:
 * Handle final ACK and remove connection
 * =====================================
 */
static void tcp_handle_last_ack(
    tcp_connection_t *conn,
    const tcp_header_t *tcp)
{
    /*
     * We are waiting for the client
     * to acknowledge our FIN.
     */
    if (!(tcp->flags & TCP_ACK))
    {
        printf(">>> Expected final ACK\n");
        return;
    }

    uint32_t received_ack =
        ntohl(tcp->ack);

    printf(">>> Final ACK Received\n");

    printf(
        ">>> Received ACK = %u\n",
        received_ack);

    printf(
        ">>> Expected ACK = %u\n",
        conn->send_next);

    /*
     * Verify that the client ACKed
     * the server FIN.
     */
    if (received_ack != conn->send_next)
    {
        printf(">>> Invalid final ACK\n");
        return;
    }

    printf(
        ">>> Server FIN acknowledged\n");

    /*
     * Phase 7E:
     * Close the connection.
     */
    conn->state = TCP_CLOSED;

    printf(
        ">>> TCP State changed: CLOSED\n");

    /*
     * Remove from connection table.
     */
    connection_remove(conn);

    printf(
        ">>> Connection removed\n");
}

static void tcp_state_machine(
    int tun_fd,
    tcp_connection_t *conn,
    ipv4_header_t *ip,
    tcp_header_t *tcp)
{
    switch (conn->state)
    {
        case TCP_SYN_RECEIVED:

            tcp_handle_syn_received(
                conn,
                tcp);

            break;


        case TCP_ESTABLISHED:

            tcp_handle_established(
                tun_fd,
                conn,
                ip,
                tcp);

            break;


        case TCP_LAST_ACK:

            tcp_handle_last_ack(
                conn,
                tcp);

            break;


        default:

            break;
    }
}

static uint32_t tcp_available_send_window(
    tcp_connection_t *conn)
{
    if (conn == NULL)
        return 0;

    uint32_t bytes_in_flight =
        retransmit_bytes_in_flight(
            &g_retransmit_queue,

            conn->src_ip,
            conn->dst_ip,

            conn->src_port,
            conn->dst_port
        );

    /*
     * Never allow unsigned underflow.
     */
    if (bytes_in_flight >=
        conn->peer_window)
    {
        return 0;
    }

    return
        conn->peer_window -
        bytes_in_flight;
}

int tcp_header_length(const tcp_header_t *tcp)
{
    return ((tcp->data_offset >> 4) & 0x0F) * 4;
}

int tcp_validate(
    const ipv4_header_t *ip,
    const uint8_t *tcp_packet,
    uint16_t tcp_packet_len)
{


    if (tcp_packet == NULL)
    {
        printf(
            "[TCP VALIDATION] NULL TCP packet\n");

        return 0;
    }

    /*
     * Minimum TCP header is 20 bytes.
     */
    if (tcp_packet_len < TCP_HEADER_SIZE)
    {
        printf(
            "[TCP VALIDATION] "
            "Packet too short: %u bytes\n",
            tcp_packet_len);

        return 0;
    }

    const tcp_header_t *tcp =
        (const tcp_header_t *)tcp_packet;

    /*
     * TCP Data Offset is the upper 4 bits
     * and is measured in 32-bit words.
     */
    uint16_t header_len =
        tcp_header_length(tcp);

    /*
     * Valid TCP header:
     *
     * Minimum = 20 bytes
     * Maximum = 60 bytes
     */
    if (header_len < TCP_HEADER_SIZE ||
        header_len > 60)
    {
        printf(
            "[TCP VALIDATION] "
            "Invalid TCP header length: %u bytes\n",
            header_len);

        return 0;
    }

    /*
     * Complete TCP header must fit inside
     * the TCP segment.
     */
    if (header_len > tcp_packet_len)
    {
        printf(
            "[TCP VALIDATION] "
            "TCP header exceeds segment length "
            "(%u > %u)\n",
            header_len,
            tcp_packet_len);

        return 0;
    }

    /*
 * Validate TCP checksum over the complete
 * TCP segment.
 */
if (!tcp_checksum_valid(
        ip,
        tcp_packet,
        tcp_packet_len))
{
    printf(
        "[TCP VALIDATION] "
        "Checksum validation failed\n");

    return 0;
}

    return 1;
}

int tcp_is_syn(const tcp_header_t *tcp)
{
    return (tcp->flags & TCP_SYN) != 0;
}

void tcp_print(const tcp_header_t *tcp)
{
    printf("========== TCP ==========\n");

    printf("Source Port      : %u\n",
            ntohs(tcp->src_port));

    printf("Destination Port : %u\n",
            ntohs(tcp->dst_port));

    printf("Sequence Number  : %u\n",
            ntohl(tcp->seq));

    printf("Ack Number       : %u\n",
            ntohl(tcp->ack));

    printf("Header Length    : %d bytes\n",
            tcp_header_length(tcp));

    printf("Window Size      : %u\n",
            ntohs(tcp->window));

    printf("Checksum         : 0x%04X\n",
            ntohs(tcp->checksum));

    printf("Flags            : ");

    if (tcp->flags & TCP_FIN) printf("FIN ");
    if (tcp->flags & TCP_SYN) printf("SYN ");
    if (tcp->flags & TCP_RST) printf("RST ");
    if (tcp->flags & TCP_PSH) printf("PSH ");
    if (tcp->flags & TCP_ACK) printf("ACK ");
    if (tcp->flags & TCP_URG) printf("URG ");
    if (tcp->flags & TCP_ECE) printf("ECE ");
    if (tcp->flags & TCP_CWR) printf("CWR ");

    printf("\n");

    printf("=========================\n\n");
}

void tcp_process(
    int tun_fd,
    ipv4_header_t *ip,
    tcp_header_t *tcp)
{
    tcp_print(tcp);

    tcp_connection_t *conn =
        connection_find(
            ip->src_ip,
            ip->dst_ip,
            ntohs(tcp->src_port),
            ntohs(tcp->dst_port));

    if (conn == NULL)
    {
        printf(">>> New TCP Connection\n");

        conn = connection_create(
            ip->src_ip,
            ip->dst_ip,
            ntohs(tcp->src_port),
            ntohs(tcp->dst_port));

        if (conn == NULL)
        {
            printf("Connection table full!\n");
            return;
        }
    }
   else
{
    printf(">>> Existing TCP Connection\n");
}

/*
 * Update the peer's advertised receive window.
 *
 * This tells us how many bytes the peer can
 * currently receive.
 */
conn->peer_window = ntohs(tcp->window);

printf(
    ">>> Peer Window = %u bytes\n",
    conn->peer_window);

/*
 * Process SYN
 */
    if (tcp_is_syn(tcp))
    {
        printf(">>> SYN Detected\n");

        conn->client_seq = ntohl(tcp->seq);
        conn->recv_next  = conn->client_seq + 1;

        printf("Client ISN : %u\n", conn->client_seq);
        printf("Recv Next  : %u\n", conn->recv_next);

        /*
         * Build reply packet
         */
        packet_t reply;

        packet_init(&reply);

        ipv4_builder_build_reply(
            &reply,
            conn);

        tcp_builder_build_syn_ack(
            &reply,
            conn);

        tcp_builder_finalize(
            &reply,
            NULL,
            0);

        printf(">>> Sending SYN-ACK (%u bytes)\n",
               packet_length(&reply));

        ssize_t written =
            write(
                tun_fd,
                packet_buffer(&reply),
                packet_length(&reply));

        if (written < 0)
        {
            perror("write");
        }
        else
        {
            printf(">>> SYN-ACK Sent\n");
        }
        return;
    }

    printf("Current TCP State : %d\n\n",
           conn->state);

    connection_print_all();

    tcp_state_machine(tun_fd, conn, ip, tcp);
}

int tcp_is_fin(const tcp_header_t *tcp)
{
    if (tcp == NULL)
        return 0;

    return (tcp->flags & TCP_FIN) != 0;
}

int tcp_checksum_valid(
    const ipv4_header_t *ip,
    const uint8_t *tcp_segment,
    uint16_t tcp_segment_length)
{
    if (ip == NULL ||
        tcp_segment == NULL ||
        tcp_segment_length < TCP_HEADER_SIZE)
    {
        return 0;
    }

    /*
     * For validation, calculate the checksum over
     * the complete received TCP segment, including
     * the checksum field.
     *
     * A valid one's-complement checksum should
     * produce zero.
     */
    return tcp_checksum(
        ip,
        tcp_segment,
        tcp_segment_length) == 0;
}

