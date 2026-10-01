# Mini TCP/IP Stack

A small educational TCP/IP stack implemented from scratch in **C for Linux**.

The project uses a Linux **TUN interface** as its virtual network device and implements core IPv4/TCP behavior directly at the packet level, including TCP connection establishment, state management, HTTP request handling, flow-control checks, packet validation, and retransmission support.

> **Goal:** learn how TCP/IP works below the socket API by building and testing the protocol stack directly over packets.

## Features

### IPv4

- IPv4 packet parsing
- IPv4 packet construction
- IPv4 header validation
- IPv4 header checksum generation and verification
- Packet-length and header-length validation
- IPv4 version validation

### TCP

- TCP header parsing and construction
- Three-way handshake
- SYN / SYN-ACK / ACK processing
- TCP connection table
- TCP state-machine handling
- Initial sequence number handling
- Wraparound-aware sequence-number comparisons
- Duplicate and out-of-order packet handling
- TCP checksum generation and validation
- Receive-window handling
- Send-window / bytes-in-flight checks
- FIN processing and connection cleanup

### Reliability

- Retransmission queue
- Tracking of unacknowledged TCP data
- ACK-based retransmission-queue cleanup
- Retransmission timeout handling
- Packet retransmission support

### HTTP

- TCP payload buffering
- HTTP request parsing
- HTTP `GET` request handling
- HTTP response generation
- HTTP response transmission over TCP
- Connection termination after the HTTP exchange

### Validation & Testing

- TCP sequence-number unit tests
- IPv4 validation tests
- TCP validation tests
- Packet-builder tests
- Checksum validation
- Malformed-packet handling
- Wireshark / tshark packet-level validation
- End-to-end TCP handshake testing
- HTTP request/response testing
- Retransmission behavior testing

## Requirements

- Linux
- GCC
- GNU Make
- Root privileges for TUN interface setup
- `curl` for HTTP testing
- Wireshark or `tshark` for packet inspection

## Build

Clone the repository and build the stack:

```bash
git clone https://github.com/tejamuppidi/network-stack.git
cd network-stack
make
```

To rebuild from a clean state:

```bash
make clean
make
```

## Run

Start the stack with:

```bash
sudo ./network_stack
```

The application automatically creates/configures the TUN interface used by the stack:

```text
Interface: tun0
Address:   10.0.0.1/24
```

Verify the interface:

```bash
ip addr show tun0
```

The stack then waits for IPv4/TCP packets.

## HTTP Testing

The current implementation can be tested using `curl`:

```bash
curl -v --connect-timeout 5 http://10.0.0.2
```

A successful test exercises the TCP handshake, HTTP request processing, HTTP response transmission, ACK processing, and TCP connection termination.

`curl` is only used as a convenient HTTP client for testing. It is **not part of the TCP/IP stack itself**.

## Packet Capture

Use `tshark` to inspect packets exchanged through the TUN interface:

```bash
sudo tshark -i tun0 \
-f "host 10.0.0.1 and host 10.0.0.2" \
-Y "tcp" \
-T fields \
-e frame.number \
-e frame.time_relative \
-e ip.src \
-e ip.dst \
-e tcp.srcport \
-e tcp.dstport \
-e tcp.flags.str \
-e tcp.seq \
-e tcp.ack \
-e tcp.len
```

This can be used to verify:

- TCP three-way handshake
- Sequence and acknowledgment numbers
- TCP flags
- TCP payload lengths
- HTTP data exchange
- FIN/ACK connection termination
- Retransmitted segments

## Automated Tests

Run the test suite with:

```bash
make test
```

The test suite includes TCP sequence-number tests and TCP validation tests.

The sequence-number tests cover:

- Normal sequence ordering
- Equal sequence numbers
- Duplicate packets
- Future/out-of-order sequences
- Sequence-number wraparound
- Wraparound equality

The TCP validation tests cover:

- Valid TCP packets
- Packets shorter than the minimum TCP header
- Invalid TCP header lengths
- Headers larger than the segment
- Invalid TCP checksums
- NULL packet handling

## Packet Validation

Incoming packets are validated before normal protocol processing.

### IPv4 validation

The IPv4 validator checks:

- Packet pointer validity
- Minimum packet size
- IPv4 version
- Header length
- Total packet length
- Header checksum

Invalid packets are rejected before further processing.

### TCP validation

The TCP validator checks:

- Packet pointer validity
- Minimum TCP segment size
- TCP data-offset/header length
- Header bounds
- TCP checksum

This prevents malformed TCP segments from reaching the connection-management and application layers.

## Retransmission

The stack maintains a retransmission queue for transmitted TCP data that has not yet been acknowledged.

When an ACK covering queued data is received, the corresponding entry can be removed from the retransmission queue.

The implementation also includes timeout-based retransmission handling for unacknowledged data.

## Project Structure

```text
network-stack/
├── include/
│   ├── checksum.h
│   ├── connection.h
│   ├── http.h
│   ├── ipv4.h
│   ├── ipv4_builder.h
│   ├── packet.h
│   ├── retransmission.h
│   ├── tcp.h
│   ├── tcp_builder.h
│   ├── tcp_sequence.h
│   └── tun.h
│
├── src/
│   ├── checksum.c
│   ├── connection.c
│   ├── http.c
│   ├── ipv4.c
│   ├── ipv4_builder.c
│   ├── main.c
│   ├── packet.c
│   ├── retransmission.c
│   ├── tcp.c
│   ├── tcp_builder.c
│   ├── tcp_sequence.c
│   └── tun.c
│
├── tests/
│   ├── test_packet_builder.c
│   ├── test_tcp_validation.c
│   └── test_validation.c
│
├── Makefile
└── README.md
```

## Design Scope

This project intentionally focuses on a small, understandable subset of networking protocols rather than attempting to reproduce the complete Linux networking stack.

The implementation is primarily intended for:

- Networking education
- TCP/IP protocol experimentation
- Packet-level debugging
- Systems programming practice
- Understanding TCP state transitions
- Exploring reliability mechanisms

## Limitations

The current implementation does not attempt to provide full production-grade TCP/IP functionality.

Examples of functionality outside the current scope include:

- Full IPv6 support
- UDP
- ICMP
- ARP/Ethernet processing
- Multi-interface routing
- Full TCP congestion control
- Selective acknowledgments (SACK)
- Complete TCP option negotiation
- Production-grade TCP performance
- Full HTTP protocol support
- General Internet traffic forwarding

## Future Work

Potential extensions include:

- Multiple simultaneous TCP connections
- TCP segmentation and reassembly
- More complete TCP option handling
- TIME-WAIT handling
- Improved retransmission backoff
- UDP support
- ICMP support
- IPv6 support
- Routing and forwarding
- Fuzz testing
- Performance benchmarking
- Broader protocol and integration testing

## License

This project is released under the **MIT License**.

## Acknowledgements

This project is intended as a learning and experimentation environment for understanding the internal behavior of TCP/IP networking on Linux.
