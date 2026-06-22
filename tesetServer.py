import socket

host = "127.0.2.1"
port = 1111

request = (
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "\r\n"
)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((host, port))
    sock.sendall(request.encode())

    response = b""
    while True:
        data = sock.recv(4096)
        if not data:
            break
        response += data

print(response.decode(errors="replace"))