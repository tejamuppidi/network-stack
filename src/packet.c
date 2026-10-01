#include "packet.h"

#include <string.h>

void packet_init(packet_t *pkt)
{
    memset(pkt->data, 0, sizeof(pkt->data));
    pkt->length = 0;
}

uint8_t *packet_buffer(packet_t *pkt)
{
    return pkt->data;
}

uint16_t packet_length(packet_t *pkt)
{
    return pkt->length;
}

void packet_set_length(packet_t *pkt, uint16_t len)
{
    pkt->length = len;
}
