CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = network_stack

SRC = \
    src/main.c \
    src/tun.c \
    src/packet.c \
    src/ipv4.c \
    src/ipv4_builder.c \
    src/tcp.c \
    src/tcp_builder.c \
    src/checksum.c \
    src/connection.c \
    src/tcp_sequence.c \
    src/http.c \
    src/retransmission.c

OBJ = $(SRC:.c=.o)


# =========================
# Unit Tests
# =========================

TEST_TARGETS = \
    test_tcp_sequence \
    test_tcp_validation


# TCP sequence number tests
TEST_SEQUENCE_SRC = \
    src/test_tcp_sequence.c \
    src/tcp_sequence.c


# IPv4/TCP validation tests
TEST_VALIDATION_SRC = \
    tests/test_tcp_validation.c \
    src/packet.c \
    src/ipv4.c \
    src/tcp.c \
    src/ipv4_builder.c \
    src/tcp_builder.c \
    src/checksum.c \
    src/connection.c \
    src/tcp_sequence.c \
    src/http.c \
    src/retransmission.c


# =========================
# Main Build
# =========================

all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)


# =========================
# Test Builds
# =========================

test_tcp_sequence: $(TEST_SEQUENCE_SRC)
	$(CC) $(CFLAGS) -o test_tcp_sequence $(TEST_SEQUENCE_SRC)


test_tcp_validation: $(TEST_VALIDATION_SRC)
	$(CC) $(CFLAGS) -o test_tcp_validation $(TEST_VALIDATION_SRC)


# =========================
# Run All Tests
# =========================

test: $(TEST_TARGETS)
	./test_tcp_sequence
	./test_tcp_validation


# =========================
# Compile Source Files
# =========================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# =========================
# Clean
# =========================

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_TARGETS)
