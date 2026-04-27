#include "server.hpp"
#include <iostream>

Data::Data()
{
    rspEnd = true;
    maxSend = 0;
    sendNext = true;
    reqEnd = "\r\n\r\n";
    recvSize = 0;
    recvNum = 0;
}

Server::Server()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cout << "error in setsockopt\n";

    len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3050);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr *)&addr, len);
    listen(server_fd, 10);

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    fds.push_back(server_poll);

    data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());

    conf.config_file();

    clientId = 0;
    A = 0;
}


void Server::acceptNewCleint(pollfd &p, size_t &i)
{
    int client_fd = accept(server_fd, (sockaddr *)&addr, &len);
    if (client_fd != -1)
    {
        pollfd client_poll;
        client_poll.fd = client_fd;
        client_poll.events = POLLIN;
        fds.push_back(client_poll);

        recs.push_back(Record());
        req.push_back(Request());
        data.push_back(Data());
    }
    else if (client_fd < 0)
    {
        close(p.fd);
        fds.erase(fds.begin() + i);
        return ;
    }
    // p.revents = 0;
    i++;
}

bool Server::receiveRequest(pollfd &p, size_t &i)
{
    char buffer[1024] = {0};
    int bytes = recv(p.fd, buffer, 1024, 0);
    if (bytes > 0)
    {
        clientId = i;
        readRequest(req, clientId, buffer, bytes, conf);
        std::cout << "WAW" << req[i].status << "\n\n\n";
        if (req[clientId].method == "GET" && (req[clientId].clientState == CLIENT_ON || req[clientId].reqState == ONE_ST))
        {
            std::cout << "mradoun raha had chi bazaf \n\n";
            p.events |= POLLOUT;
        }
        return (true);
    }
    else
    {
        close(p.fd);
        fds.erase(fds.begin() + i);
        return (false);
    }
}


bool Server::sendRespond(pollfd &p, size_t &i)
{
    if (data[i].sendNext)
    {
        data[i].response = respond(recs[i], conf, req[i]);
        if (recs[i].status == CLOSED_RS || recs[i].status == ERROR_RS)
        {
            req[i] = Request();
            recs[i] = Record();
        }
    }
    int sent = send(p.fd, data[i].response.c_str(), data[i].response.size(), 0);
    std::cout << "send = " << sent<< "\n\n";
    if (sent > 0)
    {
        data[i].response = data[i].response.substr(sent);
        data[i].sendNext = data[i].response.empty();
    }
    if (sent <= 0)
    {
        close(p.fd);
        fds.erase(fds.begin() + i);
        return (false);
    }
    if (data[i].sendNext)
        p.events &= ~POLLOUT;
    return (true);
}



int main()
{
    Server obj;

    while (true)
    {
        if (poll(obj.fds.data(), obj.fds.size(), -1) == -1)
        {
            std::cout << "poll error\n";
            continue;
        }
        for (size_t i = 0; i < obj.fds.size();)
        {
            pollfd &p = obj.fds[i];

            if (p.fd == obj.server_fd && (p.revents & POLLIN))
            {
                obj.acceptNewCleint(p, i);
                continue ;
            }
            if (p.revents & POLLIN)
            {

                if (!obj.receiveRequest(p, i))
                    continue;

                std::cout << "path = " << obj.req[i].path << "number : " << obj.req[i].status << "\n\n"; 

            }
            if (p.revents & POLLOUT)
            {
                std::cout << "hello in sevdrespond\n\n";
                if (!obj.sendRespond(p, i))
                {
                    continue ;
                }
                 std::cout << "bay kkkkk sevdrespond\n\n";
            }
            if ((p.revents & (POLLHUP | POLLERR)))
            {
                std::cout << "close fd\n\n";
                close(p.fd);
                obj.fds.erase(obj.fds.begin() + i);
                continue;
            }
            i++;
        }
    }
    close(obj.server_fd);
}

