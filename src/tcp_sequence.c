#include "tcp_sequence.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/*
 * Generate a simple Initial Sequence Number.
 *
 * This is suitable for our educational TCP stack.
 * Production TCP stacks use stronger ISN generation.
 */
static int initialized = 0;

uint32_t tcp_generate_isn(void)
{
    if (!initialized)
    {
        srand((unsigned)time(NULL));
        initialized = 1;
    }

    return (uint32_t)rand();
}


/*
 * Wraparound-safe TCP sequence comparisons.
 *
 * TCP sequence numbers are 32-bit values and
 * eventually wrap from UINT32_MAX back to 0.
 */

int tcp_seq_lt(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) < 0;
}

int tcp_seq_gt(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) > 0;
}

int tcp_seq_le(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) <= 0;
}

int tcp_seq_ge(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) >= 0;
}
