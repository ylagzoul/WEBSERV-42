#include "server.hpp"


// Server::Server()
// {
//     rspEnd = true;
//     sendNum = 0;
//     maxSend = 0;
//     sendNext = true;
//     reqEnd = "\r\n\r\n";
//     recvSize = 0;
//     recvNum = 0;
// }

int main()
{
    Config conf;
    Server obj;
    std::string response;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int n;

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

    Data dt;
    obj.data.push_back(dt);

    while (true)
    {
        poll(fds.data(),fds.size(),-1);
        for ( obj.i = 0;obj.i < fds.size();obj.i++)
        {
            if (fds[obj.i].revents & (POLLIN | POLLOUT | POLLHUP))
            {
                if (fds[obj.i].fd == server_fd)
                {
                    int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
                    pollfd client_poll;
                    client_poll.fd = client_fd;
                    client_poll.events = POLLIN;
                    fds.push_back(client_poll);
                    obj.data.push_back(dt);
                }
                else
                {
                    if (fds[obj.i].revents & POLLIN)
                    {
                        std::string buffer;
                        do
                        {
                            char cpyBuffer[1024] = {0};
                            obj.data[obj.i].recvNum = recv(fds[obj.i].fd, cpyBuffer, 1024, 0);
                            if (obj.data[obj.i].recvNum > 0)
                            {
                                std::string str(cpyBuffer,obj.data[obj.i].recvNum);
                                if (str.substr(obj.data[obj.i].recvNum - 2) != obj.data[obj.i].reqEnd)
                                {
                                    obj.data[obj.i].recvNext = false;
                                    //or function mradouan();
                                    buffer += str;
                                }
                                else
                                    buffer = str;
                            }
                            else if (obj.data[obj.i].recvNum == 0)
                            {
                                obj.data[obj.i].recvNext = true;
                                fds[obj.i].events = POLLHUP;
                            }
                            else
                            {
                                obj.data[obj.i].recvNext = true;
                                fds[obj.i].revents = POLLERR;
                            }
                        } while (obj.data[obj.i].recvNext == false);
                        //function mradouan();
                        std::cout << "Request:\n" << buffer << std::endl;
                    }
                    else if ((fds[obj.i].revents & POLLOUT) && (obj.data[obj.i].rspEnd))
                    { 
                        response = respond(obj, conf); 
                        do
                        {
                            obj.data[obj.i].sendNum += send(fds[obj.i].fd, response.substr(obj.data[obj.i].sendNum).c_str(), response.size()-obj.data[obj.i].sendNum, 0);
                            if (n > 0)
                            {
                                if (obj.data[obj.i].sendNum != obj.data[obj.i].maxSend)
                                    obj.data[obj.i].sendNext = false;
                                else
                                    obj.data[obj.i].sendNext = true;
                            }
                            else if (obj.data[obj.i].sendNum == -1)
                            {
                                fds[obj.i].revents = POLLERR;
                                obj.data[obj.i].sendNext = true;
                                std::cout << "Send error\n";
                            }
                        } while (obj.data[obj.i].sendNext == false);
                        obj.data[obj.i].sendNum = 0;
                    }
                    else
                    {
                        close(fds[obj.i].fd); 
                        fds.erase(fds.begin() + obj.i);
                        std::cout << "The connection was cut off";
                    }
                }
            }
            else if (fds[obj.i].revents & POLLERR)
            {
                close(fds[obj.i].fd);
                std::cout << "Socket error\n";
            }
        }
    }

    close(server_fd);
}
