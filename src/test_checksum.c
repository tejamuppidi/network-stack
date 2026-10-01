#include <stdio.h>

#include "checksum.h"

int main()
{
    unsigned char header[20] =
    {
        0x45,0x00,
        0x00,0x54,
        0x00,0x00,
        0x40,0x00,
        0x40,
        0x01,
        0x00,0x00,
        192,168,1,10,
        8,8,8,8
    };

    uint16_t csum = ipv4_checksum(header,20);

    printf("Checksum = 0x%04X\n",csum);

    return 0;
}
