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
    server_fd = socket(AF_INET, SOCK_STREAM, 0);//NON BLOCKING AND 3LACH AND CLOSE ON EXECV

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cerr << "error in setsockopt\n";

    len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3050);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&addr, len) < 0)
    {
        exit(1);
    }
    if (listen(server_fd, 10) < 0)
    {
        exit(1);
    }

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0;
    fds.push_back(server_poll);

    data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());

    conf.config_file();
    clientId = 0;
}

void Server::close_client(pollfd &p, int i)
{
    close(p.fd);
    fds.erase(fds.begin() + i);

    req[i].clean();
    req.erase(req.begin() + i); // iteration problem

    recs[i].clean();
    recs.erase(recs.begin() + i);

    data.erase(data.begin() + i);
}

void Server::acceptNewCleint(pollfd &p, size_t &i)
{
    int client_fd = accept(server_fd, (sockaddr *)&addr, &len);
    if (client_fd != -1)
    {
        pollfd client_poll;
        client_poll.fd = client_fd;
        client_poll.events = POLLIN;
        client_poll.revents = 0;
        fds.push_back(client_poll);
        recs.push_back(Record());
        req.push_back(Request());
        data.push_back(Data());
    }
    else if (client_fd < 0)
    {
        std::cout << "close in accept\n";
        close_client(p, i);
        return;
    }
    i++;
}


bool Server::receiveRequest(pollfd &p, size_t &i, SessionsList &sList)
{
    char buffer[8192] = {0};
    int bytes = recv(p.fd, buffer, 8192, 0);
    if (bytes > 0)
    {
        clientId = i;
        readRequest(req, clientId, buffer, bytes, conf, sList);
        // if (!req[i].cookies.empty()) // Mat9issch hadchi
        // {
        //     cout << "SS\n";
        //     cout << "--> " << req[i].cookies["session_id"] << std::endl;
        //     cout << std::endl;
        // }
        // cout << req[i].clientState << std::endl;
        // cout << req[i].status << std::endl;

        if (req[clientId].clientState == CLIENT_ON)
            p.events |= POLLOUT;
        return (true);
    }
    else if (bytes == 0)
    {
        close_client(p, i);

        return (false);
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        else
        {
            close_client(p, i);

            return (false);
        }
    }
}

bool Server::sendRespond(pollfd &p, size_t &i)
{
    if (data[i].sendNext)
    {

        data[i].response = respond(p, recs[i].data, recs[i], conf, req[i]);
        if (data[i].response.empty() && recs[i].status == ACTIVE_RS)
            return (true);
    }
    int sent = send(p.fd, data[i].response.c_str(), data[i].response.size(), MSG_NOSIGNAL);
    recs[i].number += sent;
    if (sent > 0)
    {
        data[i].response = data[i].response.substr(sent);
        data[i].sendNext = data[i].response.empty();
    }
    if (sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return (true);
        }
        else
        {

            close_client(p, i);
            return (false);
        }
    }
    if ((data[i].sendNext && recs[i].status == CLOSED_RS) || recs[i].status == ERROR_RS)
    {
        p.events &= ~POLLOUT;
    }
    return (true);
}

int main(/*int arc,char *arv[]*/)
{
    // (void)arv;
    // if (arc != 2)
    // {
    // std::cout << "erorr in inpout arguments" << std::endl;
    //     return (1);
    // }

    Server       obj;
    SessionsList sList;

    while (true)
    {
        if (poll(obj.fds.data(), obj.fds.size(), -1) == -1)// MOCHKIL F TIMEOUT!!!!! 3LA 7SAB HANDLE DISCONNECTION DYAL CLIENT
        {
            std::cout << "poll error\n";
            continue;
        }
        for (size_t i = 0; i < obj.fds.size();)
        {

            pollfd &p = obj.fds[i];

            if (p.fd == obj.server_fd && (p.revents & POLLIN))//CHECK ALL SERVER FDS
            {
                obj.acceptNewCleint(p, i);
                continue;
            }
            if (p.revents & POLLIN)
            {
                if (!obj.receiveRequest(p, i, sList))
                    continue;
            }
            if (p.revents & POLLOUT)
            {
                if (!obj.sendRespond(p, i))
                {
                    continue;
                }
            }
            if ((p.revents & (POLLHUP | POLLERR)))
            {
                obj.close_client(p, i);
                continue;
            }
            i++;
        }
    }
    close(obj.server_fd);
}

