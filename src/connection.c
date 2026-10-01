#include "connection.h"
#include "tcp_sequence.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

/* Global Connection Table */
static tcp_connection_t connections[MAX_CONNECTIONS];


/* Initialize the table */
void connection_init(void)
{
    memset(connections, 0, sizeof(connections));
}


/* Find an existing connection */
tcp_connection_t *connection_find(
    uint32_t src_ip,
    uint32_t dst_ip,
    uint16_t src_port,
    uint16_t dst_port)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (!connections[i].in_use)
            continue;

        if (connections[i].src_ip == src_ip &&
            connections[i].dst_ip == dst_ip &&
            connections[i].src_port == src_port &&
            connections[i].dst_port == dst_port)
        {
            return &connections[i];
        }
    }

    return NULL;
}


/* Create a new connection */
tcp_connection_t *connection_create(
    uint32_t src_ip,
    uint32_t dst_ip,
    uint16_t src_port,
    uint16_t dst_port)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (connections[i].in_use)
    continue;

/*
 * Clear the entire connection slot.
 *
 * This ensures no old payload data remains
 * when a previously used slot is reused.
 */
memset(
    &connections[i],
    0,
    sizeof(tcp_connection_t));


        connections[i].in_use = 1;

        connections[i].src_ip = src_ip;
        connections[i].dst_ip = dst_ip;

        connections[i].src_port = src_port;
        connections[i].dst_port = dst_port;

        connections[i].server_seq = tcp_generate_isn();

connections[i].send_next =
    connections[i].server_seq + 1;

connections[i].recv_next = 0;

/*
 * Peer window is learned from
 * incoming TCP packets.
 */
connections[i].peer_window = 0;

/*
 * No application data buffered yet.
 */
connections[i].recv_buffer_len = 0;

connections[i].state =
    TCP_SYN_RECEIVED;
        return &connections[i];
    }

    return NULL;
}


/* Remove a connection */
void connection_remove(tcp_connection_t *conn)
{
    if (conn == NULL)
        return;

    memset(conn, 0, sizeof(tcp_connection_t));
}


/* Print all active connections */
void connection_print_all(void)
{
    struct in_addr src;
    struct in_addr dst;

    printf("\n========== TCP Connection Table ==========\n");

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (!connections[i].in_use)
            continue;

        src.s_addr = connections[i].src_ip;
        dst.s_addr = connections[i].dst_ip;

        printf("Connection %d\n", i);

        printf(" SRC IP      : %s\n", inet_ntoa(src));
        printf(" DST IP      : %s\n", inet_ntoa(dst));

        printf(" SRC Port    : %u\n", connections[i].src_port);
        printf(" DST Port    : %u\n", connections[i].dst_port);

        printf(" CLIENT ISN  : %u\n", connections[i].client_seq);
        printf(" SERVER ISN  : %u\n", connections[i].server_seq);

        printf(" SEND NEXT   : %u\n", connections[i].send_next);
        printf(" RECV NEXT   : %u\n", connections[i].recv_next);

        printf(" STATE       : %d\n", connections[i].state);

        printf("------------------------------------------\n");
    }

    printf("==========================================\n");
}
