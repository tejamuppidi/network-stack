#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>

#define MAX_CONNECTIONS 1024

#define MAX_TCP_PAYLOAD_BUFFER 8192

/* TCP Connection States */
typedef enum
{
TCP_CLOSED = 0,


TCP_LISTEN,

TCP_SYN_RECEIVED,

TCP_ESTABLISHED,

/* Connection termination states */
TCP_CLOSE_WAIT,

TCP_LAST_ACK,

TCP_FIN_WAIT_1,

TCP_FIN_WAIT_2,

TCP_TIME_WAIT


} tcp_state_t;

/* Represents one TCP connection */
typedef struct
{
/* IPv4 Addresses */
uint32_t src_ip;
uint32_t dst_ip;


/* TCP Ports */
uint16_t src_port;
uint16_t dst_port;

/* Client Initial Sequence Number */
uint32_t client_seq;

/* Server Initial Sequence Number */
uint32_t server_seq;

/* Next sequence number to send */
uint32_t send_next;

/* Next sequence number expected */
uint32_t recv_next;

/* Peer's advertised TCP receive window */
uint32_t peer_window;

/* Current TCP State */
tcp_state_t state;

/* Is this slot used? */
int in_use;

uint8_t recv_buffer[MAX_TCP_PAYLOAD_BUFFER];

uint32_t recv_buffer_len;

} tcp_connection_t;

/* Initialize connection table */
void connection_init(void);

/* Find an existing connection */
tcp_connection_t *connection_find(
uint32_t src_ip,
uint32_t dst_ip,
uint16_t src_port,
uint16_t dst_port);

/* Create a new connection */
tcp_connection_t *connection_create(
uint32_t src_ip,
uint32_t dst_ip,
uint16_t src_port,
uint16_t dst_port);

/* Remove a connection */
void connection_remove(
tcp_connection_t *conn);

/* Debug */
void connection_print_all(void);

#endif

