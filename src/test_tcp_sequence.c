#include <stdio.h>
#include <stdint.h>
#include "tcp_sequence.h"

static int test(const char *name, int condition)
{
    printf("%s... ", name);

    if (condition) {
        printf("PASS\n");
        return 1;
    }

    printf("FAIL\n");
    return 0;
}

int main(void)
{
    int passed = 0;
    int total = 6;

    printf("\n====================================\n");
    printf(" TCP SEQUENCE NUMBER UNIT TESTS\n");
    printf("====================================\n\n");

    passed += test(
        "Test 1: Normal sequence ordering",
        tcp_seq_lt(100, 200) &&
        tcp_seq_gt(200, 100) &&
        tcp_seq_le(100, 200) &&
        tcp_seq_ge(200, 100)
    );

    passed += test(
        "Test 2: Equal sequence numbers",
        !tcp_seq_lt(1000, 1000) &&
        !tcp_seq_gt(1000, 1000) &&
        tcp_seq_le(1000, 1000) &&
        tcp_seq_ge(1000, 1000)
    );

    passed += test(
        "Test 3: Duplicate packet sequence",
        tcp_seq_le(5000, 5000) &&
        tcp_seq_ge(5000, 5000)
    );

    passed += test(
        "Test 4: Future/out-of-order sequence",
        tcp_seq_gt(2000, 1000) &&
        tcp_seq_lt(1000, 2000)
    );

    passed += test(
        "Test 5: TCP sequence wraparound",
        tcp_seq_gt(0, UINT32_MAX - 10) &&
        tcp_seq_lt(UINT32_MAX - 10, 0)
    );

    passed += test(
        "Test 6: Wraparound equality",
        !tcp_seq_lt(0, 0) &&
        !tcp_seq_gt(0, 0) &&
        tcp_seq_le(0, 0) &&
        tcp_seq_ge(0, 0)
    );

    printf("\n====================================\n");

    if (passed == total) {
        printf(" ALL TCP SEQUENCE TESTS PASSED\n");
    } else {
        printf(" TCP SEQUENCE TESTS: %d/%d PASSED\n", passed, total);
    }

    printf("====================================\n\n");

    return passed == total ? 0 : 1;
}
