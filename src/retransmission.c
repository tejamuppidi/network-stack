#include "retransmission.h"
#include "tcp_sequence.h"
#include "connection.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

retransmit_queue_t g_retransmit_queue;

void retransmit_init(void)
{
    retransmit_queue_init(
        &g_retransmit_queue
    );
}

void retransmit_queue_init(
    retransmit_queue_t *queue)
{
    if (queue == NULL)
        return;

    memset(queue, 0, sizeof(*queue));
}


int retransmit_store(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port,

    const uint8_t *packet,
    int packet_len,

    uint32_t seq_start,
    uint32_t seq_end)
{
    if (queue == NULL ||
        packet == NULL ||
        packet_len <= 0 ||
        packet_len > MAX_RETRANSMIT_PACKET_SIZE)
    {
        return -1;
    }


    for (int i = 0;
         i < MAX_RETRANSMIT_PACKETS;
         i++)
    {
        retransmit_entry_t *entry =
            &queue->entries[i];


        if (entry->active)
            continue;


        entry->active = 1;

        /*
 * Store TCP connection identity
 */
entry->src_ip = src_ip;
entry->dst_ip = dst_ip;

entry->src_port = src_port;
entry->dst_port = dst_port;


        memcpy(
            entry->packet,
            packet,
            packet_len
        );

        entry->packet_len = packet_len;

        entry->seq_start = seq_start;
        entry->seq_end = seq_end;

        entry->sent_time = time(NULL);

        entry->retransmit_count = 0;


        printf(
            "[RETRANSMIT] Stored packet "
            "SEQ=%u END=%u LEN=%d\n",
            entry->seq_start,
            entry->seq_end,
            entry->packet_len
        );


        return 0;
    }


    printf(
        "[RETRANSMIT] Queue full\n"
    );

    return -1;
}


void retransmit_ack(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port,

    uint32_t ack_number)
{
    if (queue == NULL)
        return;

    #if TCP_RETRANSMIT_TEST_MODE

    printf(
        "[RETRANSMIT TEST] Ignoring ACK=%u\n",
        ack_number
    );

    return;

#endif


    for (int i = 0;
         i < MAX_RETRANSMIT_PACKETS;
         i++)
    {
        retransmit_entry_t *entry =
            &queue->entries[i];


        if (!entry->active)
            continue;

        /*
 * Ignore entries belonging to
 * other TCP connections.
 */
if (entry->src_ip != src_ip ||
    entry->dst_ip != dst_ip ||
    entry->src_port != src_port ||
    entry->dst_port != dst_port)
{
    continue;
}


        /*
         * If ACK acknowledges the entire
         * sequence range of this packet,
         * remove it.
         */
       if (tcp_seq_ge(
        ack_number,
        entry->seq_end))
        {
            printf(
                "[RETRANSMIT] ACK received "
                "ACK=%u "
                "removing SEQ=%u END=%u\n",
                ack_number,
                entry->seq_start,
                entry->seq_end
            );


            memset(
                entry,
                0,
                sizeof(*entry)
            );
        }
    }
}


void retransmit_check_timeouts(
    retransmit_queue_t *queue,
    int tun_fd)
{
    if (queue == NULL)
        return;


    time_t now = time(NULL);


    for (int i = 0;
         i < MAX_RETRANSMIT_PACKETS;
         i++)
    {
        retransmit_entry_t *entry =
            &queue->entries[i];


        if (!entry->active)
            continue;

        /*
         * Do not retransmit packets for
         * connections that no longer exist.
         */
        tcp_connection_t *conn =
            connection_find(
                entry->src_ip,
                entry->dst_ip,
                entry->src_port,
                entry->dst_port
            );


        if (conn == NULL)
        {
            printf(
                "[RETRANSMIT] Connection gone "
                "removing SEQ=%u\n",
                entry->seq_start
            );


            memset(
                entry,
                0,
                sizeof(*entry)
            );


            continue;
        }


        double elapsed =
            difftime(
                now,
                entry->sent_time
            );


        if (elapsed <
            TCP_RETRANSMIT_TIMEOUT)
        {
            continue;
        }


        /*
         * Maximum retransmissions reached.
         */
        if (entry->retransmit_count >=
            TCP_MAX_RETRANSMISSIONS)
        {
            printf(
                "[RETRANSMIT] FAILED "
                "SEQ=%u after %d retries\n",
                entry->seq_start,
                entry->retransmit_count
            );


            memset(
                entry,
                0,
                sizeof(*entry)
            );

            continue;
        }


        printf(
            "[RETRANSMIT] TIMEOUT "
            "SEQ=%u retry=%d\n",
            entry->seq_start,
            entry->retransmit_count + 1
        );


        ssize_t written =
            write(
                tun_fd,
                entry->packet,
                entry->packet_len
            );


       if (written != entry->packet_len)
{
    if (written < 0)
    {
        perror("[RETRANSMIT] write");
    }
    else
    {
        fprintf(
            stderr,
            "[RETRANSMIT] Partial write: %zd/%d bytes\n",
            written,
            entry->packet_len
        );
    }

    continue;
}

        entry->retransmit_count++;

        entry->sent_time = now;
    }
}

uint32_t retransmit_bytes_in_flight(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port)
{
    if (queue == NULL)
        return 0;

    uint32_t bytes_in_flight = 0;

    for (int i = 0;
         i < MAX_RETRANSMIT_PACKETS;
         i++)
    {
        retransmit_entry_t *entry =
            &queue->entries[i];

        if (!entry->active)
            continue;

        /*
         * Only count packets belonging
         * to this TCP connection.
         */
        if (entry->src_ip != src_ip ||
            entry->dst_ip != dst_ip ||
            entry->src_port != src_port ||
            entry->dst_port != dst_port)
        {
            continue;
        }

        /*
         * Sequence range represents the
         * TCP sequence space occupied by
         * this packet.
         */
        bytes_in_flight +=
            entry->seq_end -
            entry->seq_start;
    }

    return bytes_in_flight;
}

void retransmit_print(
    retransmit_queue_t *queue)
{
    if (queue == NULL)
        return;


    printf(
        "\n===== Retransmission Queue =====\n"
    );


    for (int i = 0;
         i < MAX_RETRANSMIT_PACKETS;
         i++)
    {
        retransmit_entry_t *entry =
            &queue->entries[i];


        if (!entry->active)
            continue;


        printf(
            "[%d] "
            "SEQ=%u "
            "END=%u "
            "LEN=%d "
            "RETRIES=%d\n",

            i,

            entry->seq_start,
            entry->seq_end,

            entry->packet_len,

            entry->retransmit_count
        );
    }


    printf(
        "================================\n"
    );
}
