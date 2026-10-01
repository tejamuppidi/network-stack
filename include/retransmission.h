#ifndef RETRANSMISSION_H
#define RETRANSMISSION_H

#include <stdint.h>
#include <time.h>

#define MAX_RETRANSMIT_PACKETS 16
#define MAX_RETRANSMIT_PACKET_SIZE 2048

/*
 * Initial retransmission timeout in seconds.
 *
 * We will later improve this with dynamic RTO calculation.
 */
#define TCP_RETRANSMIT_TIMEOUT 3

/*
 * Maximum number of retransmissions before
 * considering the packet failed.
 */
#define TCP_MAX_RETRANSMISSIONS 5

/*
 * Phase 9E testing only.
 *
 * When set to 1, incoming ACKs will not
 * remove retransmission entries.
 */
#define TCP_RETRANSMIT_TEST_MODE 0

typedef struct
{
    int active;

    /*
     * Connection identity.
     *
     * Used to ensure ACKs only acknowledge
     * packets belonging to the same TCP flow.
     */
    uint32_t src_ip;
    uint32_t dst_ip;

    uint16_t src_port;
    uint16_t dst_port;


    /* Complete packet copy */

    /* Complete packet copy */
    uint8_t packet[MAX_RETRANSMIT_PACKET_SIZE];

    int packet_len;

    /*
     * TCP sequence range covered by this packet.
     *
     * Example:
     * seq_start = 1000
     * seq_end   = 1100
     */
    uint32_t seq_start;
    uint32_t seq_end;

    /* Time when packet was last sent */
    time_t sent_time;

    /* Number of retransmissions */
    int retransmit_count;

} retransmit_entry_t;


/*
 * Retransmission queue.
 *
 * For now this is global/simple.
 * Later we can make it per connection.
 */
typedef struct
{
    retransmit_entry_t entries[MAX_RETRANSMIT_PACKETS];

} retransmit_queue_t;


/* Initialize retransmission queue */
void retransmit_queue_init(
    retransmit_queue_t *queue
);


/*
 * Store a packet that requires acknowledgement.
 *
 * Returns:
 *  0  success
 * -1  failure
 */
int retransmit_store(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port,

    const uint8_t *packet,
    int packet_len,

    uint32_t seq_start,
    uint32_t seq_end
);


/*
 * Process an incoming ACK.
 *
 * All packets completely acknowledged by ack_number
 * are removed.
 */
void retransmit_ack(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port,

    uint32_t ack_number
);


/*
 * Check for packets whose retransmission timer expired.
 *
 * tun_fd is used to send the stored packet again.
 */
void retransmit_check_timeouts(
    retransmit_queue_t *queue,
    int tun_fd
);

/*
 * Return the number of TCP payload bytes
 * sent but not yet acknowledged for a
 * specific connection.
 */
uint32_t retransmit_bytes_in_flight(
    retransmit_queue_t *queue,

    uint32_t src_ip,
    uint32_t dst_ip,

    uint16_t src_port,
    uint16_t dst_port
);

/* Print retransmission queue for debugging */
void retransmit_print(
    retransmit_queue_t *queue
);


/*
 * Global retransmission queue used by
 * the mini TCP/IP stack.
 */
extern retransmit_queue_t g_retransmit_queue;


/* Initialize global retransmission system */
void retransmit_init(void);

#endif
