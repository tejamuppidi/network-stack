#ifndef TCP_SEQUENCE_H
#define TCP_SEQUENCE_H

#include <stdint.h>

/* Generate TCP Initial Sequence Number */
uint32_t tcp_generate_isn(void);

/*
 * Wraparound-safe TCP sequence comparisons.
 */
int tcp_seq_lt(uint32_t a, uint32_t b);
int tcp_seq_gt(uint32_t a, uint32_t b);
int tcp_seq_le(uint32_t a, uint32_t b);
int tcp_seq_ge(uint32_t a, uint32_t b);

#endif
