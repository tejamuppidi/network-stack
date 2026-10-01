#include "ipv4.h"
#include "checksum.h"

#include <arpa/inet.h>
#include <stdio.h>

int ipv4_header_length(const ipv4_header_t *ip)
{
    return (ip->version_ihl & 0x0F) * 4;
}

int ipv4_version(const ipv4_header_t *ip)
{
    return ip->version_ihl >> 4;
}

void ipv4_print(const ipv4_header_t *ip)
{
    struct in_addr src;
    struct in_addr dst;

    src.s_addr = ip->src_ip;
    dst.s_addr = ip->dst_ip;

    printf("========== IPv4 ==========\n");

    printf("Version        : %d\n",
            ipv4_version(ip));

    printf("Header Length  : %d bytes\n",
            ipv4_header_length(ip));

    printf("Total Length   : %u\n",
            ntohs(ip->total_length));

    printf("TTL            : %u\n",
            ip->ttl);

    printf("Protocol       : %u\n",
            ip->protocol);

    printf("Checksum       : 0x%04X\n",
            ntohs(ip->checksum));

    printf("Source         : %s\n",
            inet_ntoa(src));

    printf("Destination    : %s\n",
            inet_ntoa(dst));

    printf("==========================\n\n");
}

int ipv4_checksum_valid(
    const ipv4_header_t *ip,
    uint16_t header_len)
{
    /*
     * When the checksum is calculated over an IPv4
     * header that already contains its checksum,
     * the result must be zero.
     */
    return ipv4_checksum(ip, header_len) == 0;
}


int ipv4_validate(
    const uint8_t *packet,
    uint16_t packet_len)
{
    if (packet == NULL)
    {
        printf("[IPv4 VALIDATION] NULL packet\n");
        return 0;
    }

    if (packet_len < IPV4_HEADER_SIZE)
    {
        printf("[IPv4 VALIDATION] "
               "Packet too short: %u bytes\n",
               packet_len);
        return 0;
    }

    const ipv4_header_t *ip =
        (const ipv4_header_t *)packet;

    if (ipv4_version(ip) != IPV4_VERSION)
    {
        printf("[IPv4 VALIDATION] "
               "Unsupported IP version: %d\n",
               ipv4_version(ip));
        return 0;
    }

    uint16_t header_len =
        ipv4_header_length(ip);

    if (header_len < IPV4_HEADER_SIZE)
    {
        printf("[IPv4 VALIDATION] "
               "Invalid header length: %u\n",
               header_len);
        return 0;
    }

    if (header_len > packet_len)
    {
        printf("[IPv4 VALIDATION] "
               "Header length exceeds packet length\n");
        return 0;
    }

    uint16_t total_length =
        ntohs(ip->total_length);

    if (total_length < header_len)
    {
        printf("[IPv4 VALIDATION] "
               "Total length smaller than header\n");
        return 0;
    }

    if (total_length > packet_len)
    {
        printf("[IPv4 VALIDATION] "
               "Total length exceeds received packet "
               "(%u > %u)\n",
               total_length,
               packet_len);
        return 0;
    }

    if (!ipv4_checksum_valid(ip, header_len))
    {
        printf("[IPv4 VALIDATION] "
               "Checksum validation failed\n");
        return 0;
    }

    return 1;
}
