import socket

HOST = "localhost"
PORT = 3050

request = (
    "GET /uploads/ HTTP/1.0\r\n"
    "Host: localhost\r\n"
    "Content-Length: 4\r\n"
    "\r\n"
    "abcd"
)

with socket.create_connection((HOST, PORT)) as sock:
    sock.sendall(request.encode("ascii"))

    response = b""
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        response += chunk

print(response.decode("utf-8", errors="replace"))