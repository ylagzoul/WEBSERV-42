#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>


void Add_New_Cleint(std::vector<pollfd> &fds, int client_fd)
{
    pollfd client_poll;
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;

    fds.push_back(client_poll);
    std::cout << "new client " << std::endl;
}


int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(452);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, len);
    listen(server_fd, 5);

    std::vector<pollfd> fds;

    pollfd server_poll;

    server_poll.fd = server_fd;
    server_poll.events = POLLIN;


    fds.push_back(server_poll);
    while (true)
    {
        poll(fds.data(),fds.size(),-1);
        for (size_t i = 0;i < fds.size();i++)
        {
            if (fds[i].revents & (POLLIN | POLLOUT | POLLHUP))
            {
                std::cout << "yousenf"<< std::endl;
                if (fds[i].fd == server_fd)
                {
                    int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
                    pollfd client_poll;
                    client_poll.fd = client_fd;
                    client_poll.events = POLLIN;

                    fds.push_back(client_poll);
                    std::cout << "new client " << std::endl;
                }
                else
                {
                    if (fds[i].revents & POLLIN)
                    {
                        char buffer[1024] = {0};
                        recv(fds[i].fd, buffer, 1024, 0);
                        std::cout << "Request:\n" << buffer << std::endl;
                    }
                    else if (fds[i].revents & POLLOUT)
                    {
                        std::string body;
                        if (i == 1)
                            body = "<html><body><h1>Hello from server</h1></body></html>";
                        else if (i == 2)
                            body = "<html><body><h1>Hello from server conictino client 111</h1></body></html>";
                        else if (i == 3)
                            body = "<html><body><h1>Hello from server (.lo.) </h1></body></html>";
                        else
                            body = "<html><body><h1>Hello</h1></body></html>";

                        std::string response =
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + std::to_string(body.size()) + "\r\n"
                            "\r\n" +
                            body;
                        // res = re();
                        send(fds[i].fd, response.c_str(), response.size(), 0);
                    }
                    else
                    {
                        close(fds[i].fd); 
                        fds.erase(fds.begin() + i);
                        i--;
            
                        std::cout << "The connection was cut off";
                    }
                }
            }
            else if (fds[i].revents & POLLERR)
            {
                close(fds[i].fd);
                std::cout << "Socket error\n";
            }
        }
    }

    close(server_fd);
}