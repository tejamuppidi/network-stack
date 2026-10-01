#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>
#include <stddef.h>

#include "ipv4.h"
#include "tcp.h"

uint16_t checksum(const void *data, size_t length);

uint16_t ipv4_checksum(const void *header, size_t length);

uint16_t tcp_checksum(
    const ipv4_header_t *ip,
    const uint8_t *tcp_segment,
    uint16_t tcp_segment_length
);

#endif
