#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8001);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    const char* req =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    // Send slowly
    for (size_t i = 0; req[i] != '\0'; ++i) {
        if (send(sock, &req[i], 1, 0) != 1) {
            perror("send");
            close(sock);
            return 1;
        }
        std::cout << req [i] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }

    // Read and print response
    char buffer[4096];
    ssize_t bytes;

    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }

    if (bytes < 0)
        perror("recv");

    close(sock);
}
