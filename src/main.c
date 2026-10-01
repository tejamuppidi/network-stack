#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#include "packet.h"
#include "tun.h"
#include "ipv4.h"
#include "tcp.h"
#include "connection.h"
#include "retransmission.h"


int main(void)
{
char tun_name[IFNAMSIZ] = "tun0";

int tun_fd = tun_alloc(tun_name);

if (tun_fd < 0)
{
    perror("tun_alloc");
    return EXIT_FAILURE;
}

printf("=====================================\n");
printf("        Mini TCP/IP Stack\n");
printf("=====================================\n");
printf("Interface : %s\n", tun_name);
printf("Waiting for packets...\n\n");

packet_t packet;

packet_init(&packet);

/* Initialize TCP connection table */
connection_init();

retransmit_init();


while (1)
{
    fd_set read_fds;

    FD_ZERO(&read_fds);
    FD_SET(tun_fd, &read_fds);


    /*
     * Wake up every 100 milliseconds
     * even if no packet arrives.
     */
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;


    int ready =
        select(
            tun_fd + 1,
            &read_fds,
            NULL,
            NULL,
            &timeout
        );


    /*
     * Phase 9D:
     * Check retransmission timers
     * every loop iteration.
     */
    retransmit_check_timeouts(
        &g_retransmit_queue,
        tun_fd
    );


    if (ready < 0)
    {
        perror("select");
        break;
    }


    /*
     * No packet arrived.
     *
     * Timers were already checked above.
     */
    if (ready == 0)
    {
        continue;
    }


    /*
     * TUN interface has data.
     */
    if (!FD_ISSET(tun_fd, &read_fds))
    {
        continue;
    }


    int bytes =
        read(
            tun_fd,
            packet_buffer(&packet),
            MAX_PACKET_SIZE);

    if (bytes < 0)
    {
        perror("read");
        break;
    }

    packet_set_length(&packet, bytes);

   /*
 * Phase 8:
 * Validate the complete IPv4 packet before
 * processing any protocol fields.
 */
if (!ipv4_validate(
        packet_buffer(&packet),
        packet_length(&packet)))
{
    printf("Packet dropped by IPv4 validator\n\n");
    continue;
}

ipv4_header_t *ip =
    (ipv4_header_t *)packet_buffer(&packet);

uint16_t ip_header_len =
    ipv4_header_length(ip);
    printf("=====================================\n");
    printf("Packet Length : %u bytes\n\n",
           packet_length(&packet));

    ipv4_print(ip);

    switch (ip->protocol)
    {
        case IP_PROTO_TCP:
        {
            /*
             * The validated IPv4 total length tells us
             * the exact size of the TCP segment.
             */
            uint16_t total_length =
                ntohs(ip->total_length);

            uint16_t tcp_segment_len =
                total_length - ip_header_len;

            /*
             * The complete IPv4 packet has already
             * been validated.
             */
            if (tcp_segment_len < TCP_HEADER_SIZE)
            {
                printf(
                    "[TCP VALIDATION] "
                    "Segment too short for TCP header\n\n");

                break;
         }

        uint8_t *tcp_packet =
            (uint8_t *)ip + ip_header_len;

        if (!tcp_validate(
                ip,
                tcp_packet,
                tcp_segment_len))
        {
            printf(
                "Invalid TCP packet dropped\n\n");

            break;
        }

        tcp_header_t *tcp =
            (tcp_header_t *)tcp_packet;

        tcp_process(
            tun_fd,
            ip,
            tcp);

        break;
    }

        case IP_PROTO_UDP:
        {
            printf("UDP Packet\n\n");
            break;
        }

        case IP_PROTO_ICMP:
        {
            printf("ICMP Packet\n\n");
            break;
        }

        default:
        {
            printf("Unknown Protocol : %u\n\n",
                   ip->protocol);
            break;
        }
    }
}

close(tun_fd);

return EXIT_SUCCESS;


}

