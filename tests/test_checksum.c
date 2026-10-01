#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "checksum.h"

int main(void)
{
    printf("====================================\n");
    printf(" CHECKSUM UNIT TESTS\n");
    printf("====================================\n\n");

    const uint8_t data[] = {
        0x45, 0x00, 0x00, 0x28,
        0x12, 0x34, 0x00, 0x00,
        0x40, 0x06, 0x00, 0x00
    };

    uint16_t result = checksum(data, sizeof(data));

    printf("Checksum = 0x%04X\n", result);

    if (result == 0) {
        printf("FAIL\n");
        return 1;
    }

    printf("PASS\n");
    return 0;
}
