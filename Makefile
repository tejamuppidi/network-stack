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


# ============================================================
# Tests
# ============================================================

TEST_TARGETS = \
	test_checksum \
	test_packet_builder \
	test_tcp_sequence \
	test_tcp_validation \
	test_validation


# ============================================================
# Test Sources
# ============================================================

TEST_CHECKSUM_SRC = \
	src/test_checksum.c \
	src/checksum.c

TEST_PACKET_BUILDER_SRC = \
	tests/test_packet_builder.c \
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

TEST_TCP_SEQUENCE_SRC = \
	src/test_tcp_sequence.c \
	src/tcp_sequence.c

TEST_TCP_VALIDATION_SRC = \
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

TEST_VALIDATION_SRC = \
	tests/test_validation.c \
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


# ============================================================
# Main Build
# ============================================================

.PHONY: all test clean

all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)


# ============================================================
# Test Builds
# ============================================================

test_checksum: $(TEST_CHECKSUM_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_CHECKSUM_SRC)


test_packet_builder: $(TEST_PACKET_BUILDER_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_PACKET_BUILDER_SRC)


test_tcp_sequence: $(TEST_TCP_SEQUENCE_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_TCP_SEQUENCE_SRC)


test_tcp_validation: $(TEST_TCP_VALIDATION_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_TCP_VALIDATION_SRC)


test_validation: $(TEST_VALIDATION_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_VALIDATION_SRC)


# ============================================================
# Run Complete Test Suite
# ============================================================

test: $(TEST_TARGETS)
	@echo ""
	@echo "=============================================="
	@echo " RUNNING COMPLETE TCP/IP TEST SUITE"
	@echo "=============================================="
	@echo ""

	@echo "[1/5] Checksum tests"
	@./test_checksum

	@echo ""
	@echo "[2/5] Packet builder tests"
	@./test_packet_builder

	@echo ""
	@echo "[3/5] TCP sequence tests"
	@./test_tcp_sequence

	@echo ""
	@echo "[4/5] TCP validation tests"
	@./test_tcp_validation

	@echo ""
	@echo "[5/5] IPv4 validation tests"
	@./test_validation

	@echo ""
	@echo "=============================================="
	@echo " ALL TEST SUITES COMPLETED"
	@echo "=============================================="


# ============================================================
# Compile Source Files
# ============================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# ============================================================
# Clean
# ============================================================

clean:
	rm -f $(OBJ) \
		$(TARGET) \
		$(TEST_TARGETS)
