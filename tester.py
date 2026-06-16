#!/usr/bin/env python3

import socket

HOST = "127.0.0.1"
PORT = 3050


def send_request(raw_request):
    with socket.create_connection((HOST, PORT)) as sock:
        sock.sendall(raw_request)
        response = sock.recv(4096)
        print(response.decode("latin1", errors="replace"))


# --------------------------------------------------
# Test 1: Header too large
# --------------------------------------------------

# large_header = "A" * 100000

# request = (
#     "GET / HTTP/1.1\r\n"
#     "Host: localhost\r\n"
#     f"X-Large: {large_header}\r\n"
#     "\r\n"
# )

# print("=" * 50)
# print("TEST 1: LARGE HEADER")
# print("=" * 50)

# send_request(request.encode())


# --------------------------------------------------
# Test 2: URI too large
# --------------------------------------------------

# large_uri = "/" + ("A" * 100000)

# request = (
#     f"GET {large_uri} HTTP/1.1\r\n"
#     "Host: localhost\r\n"
#     "\r\n"
# )

# print("=" * 50)
# print("TEST 2: LARGE URI")
# print("=" * 50)

# send_request(request.encode())


# --------------------------------------------------
# Test 3: Large body
# --------------------------------------------------

body = b"A" * 1000000

headers = (
    "POST /upload HTTP/1.1\r\n"
    "Host: localhost\r\n"
    f"Content-Length: {len(body)}\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
).encode()

print("=" * 50)
print("TEST 3: LARGE BODY")
print("=" * 50)

send_request(headers + body)