#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include "server.hpp"

Server::Server()
{
    sendNum = 0;
    sendSize = 0;
    maxSend = 0;
    sendNext = true;
    reqEnd = "\r\n\r\n";
    recvSize = 0;
    recvNum = 0;
}

int re()
{
    return (800);
}


int main()
{
    std::vector<Server> data;
    int n;
    std::string response;
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

    Server obj;
    data.push_back(obj);

    while (true)
    {
        poll(fds.data(),fds.size(),-1);
        for (size_t i = 0;i < fds.size();i++)
        {
            if (fds[i].revents & (POLLIN | POLLOUT | POLLHUP))
            {
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
                        std::string buffer;
                        do
                        {
                            char cpyBuffer[1024] = {0};
                            data[i].recvNum = recv(fds[i].fd, cpyBuffer, 1024, 0);
                            if (data[i].recvNum > 0)
                            {
                                std::string str(cpyBuffer,data[i].recvNum);
                                if (str.substr(data[i].recvNum - 2) != data[i].reqEnd)
                                {
                                    data[i].recvNext = false;
                                    //or function mradouan();
                                    buffer += str;
                                }
                                else
                                    buffer = str;
                            }
                            else if (data[i].recvNum == 0)
                            {
                                data[i].recvNext = true;
                                fds[i].events = POLLHUP;
                            }
                            else
                            {
                                data[i].recvNext = true;
                                fds[i].revents = POLLERR;
                            }
                        } while (data[i].recvNext == false);
                        //function mradouan();
                        std::cout << "Request:\n" << buffer << std::endl;
                    }
                    else if ((fds[i].revents & POLLOUT) && (data[i].rspEnd))
                    {   
                        response = re(); 
                        do
                        {
                            data[i].sendNum += send(fds[i].fd, response.substr(data[i].sendNum).c_str(), response.size()-data[i].sendNum, 0);
                            if (n > 0)
                            {
                                if (data[i].sendNum != data[i].maxSend)
                                    data[i].sendNext = false;
                                else
                                    data[i].sendNext = true;
                            }
                            else if (data[i].sendNum == -1)
                            {
                                fds[i].revents = POLLERR;
                                data[i].sendNext = true;
                                std::cout << "Send error\n";
                            }
                        } while (data[i].sendNext == false);
                        data[i].sendNum = 0;
                    }
                    else
                    {
                        close(fds[i].fd); 
                        fds.erase(fds.begin() + i);
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
