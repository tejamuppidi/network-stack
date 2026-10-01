import socket
import time

HOST = "10.0.0.2"
PORT = 80

sock = socket.socket(
    socket.AF_INET,
    socket.SOCK_STREAM
)

sock.bind(("10.0.0.1", 0))

sock.connect((HOST, PORT))

request = (
    "GET / HTTP/1.1\r\n"
    "Host: 10.0.0.2\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
)

sock.sendall(request.encode())

print("[CLIENT] Request sent")

response = sock.recv(4096)

print("[CLIENT] Response received:")
print(response.decode(errors="replace"))

print("[CLIENT] Keeping connection open for 20 seconds...")

time.sleep(20)

print("[CLIENT] Closing connection")

sock.close()
