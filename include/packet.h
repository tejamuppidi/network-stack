#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define MAX_PACKET_SIZE 2048

typedef struct
{
    uint8_t data[MAX_PACKET_SIZE];
    uint16_t length;

} packet_t;

/* Allocate a new packet */

void packet_init(packet_t *pkt);

/* Return pointer to raw bytes */

uint8_t *packet_buffer(packet_t *pkt);

/* Packet length */

uint16_t packet_length(packet_t *pkt);

/* Set packet length */

void packet_set_length(packet_t *pkt, uint16_t len);

#endif
