import socket

HOST = "localhost"

PORT = 3050

request = (
    "GET / HTTP/1.1\r\n"
    f"Host: {HOST}\r\n"
    "Connection: close\r\n"
    "\r\n"
)

with socket.create_connection((HOST, PORT)) as sock:
    # Send request
    sock.sendall(request.encode())

    print("Request sent")

    # Close only the write side (send FIN)
    sock.shutdown(socket.SHUT_WR)

    print("Write side closed, waiting for response...")

    # Continue reading response
    response = b""
    while True:
        data = sock.recv(4096)
        if not data:
            break
        response += data

    print("Response received:")
    print(response.decode(errors="replace"))
