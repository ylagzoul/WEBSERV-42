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
    conf.config_file();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cerr << "error in setsockopt\n";

    len = sizeof(addr);
    addr.sin_family = AF_INET;
    // addr.sin_port = htons(conf.listen_Port[0]);
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
    index_Cleint.push_back(0);

    data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());

    clientId = 0;
}

void Server::close_client(pollfd &p, int i)
{
    close(p.fd);
    fds.erase(fds.begin() + index_Cleint[i]);

    req[index_Cleint[i]].clean();
    req.erase(req.begin() + index_Cleint[i]); // iteration problem
    
    recs[index_Cleint[i]].clean();
    recs.erase(recs.begin() + index_Cleint[i]);
    
    data.erase(data.begin() + index_Cleint[i]);
    index_Cleint.erase(index_Cleint.begin() + index_Cleint.size()-1);
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
        index_Cleint.push_back(fds.size() - 1);
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

bool Server:: receiveRequest(pollfd &p, size_t &i)
{
    char buffer[1024] = {0};
    int bytes = recv(p.fd, buffer, 1024, 0);
    if (bytes > 0)
    { 
        clientId = index_Cleint[i];
        readRequest(req, clientId, buffer, bytes, conf);

        if (req[index_Cleint[i]].path.empty())
            req[index_Cleint[i]].path = "/";
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

bool Server::sendRespond(Server &obj, pollfd &p)
{
    if (data[index_Cleint[iClnt]].sendNext)
    {

        data[index_Cleint[iClnt]].response = respond(obj, recs[index_Cleint[iClnt]].data, recs[index_Cleint[iClnt]], conf, req[index_Cleint[iClnt]]);
        if (data[index_Cleint[iClnt]].response.empty() && recs[index_Cleint[iClnt]].status == ACTIVE_RS)
            return (true);
    }
    int sent = send(p.fd, data[index_Cleint[iClnt]].response.c_str(), data[index_Cleint[iClnt]].response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    recs[index_Cleint[iClnt]].number += sent;
    if (sent > 0)
    {
        data[index_Cleint[iClnt]].response = data[index_Cleint[iClnt]].response.substr(sent);
        data[index_Cleint[iClnt]].sendNext = data[index_Cleint[iClnt]].response.empty();
    }
    if (sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return (true);
        }
        else
        {
            std::cout << "lagzouli 1111   \n\n";

            close_client(p, iClnt);
            return (false);
        }
    }
    if ((data[index_Cleint[iClnt]].sendNext && recs[index_Cleint[iClnt]].status == CLOSED_RS) || recs[index_Cleint[iClnt]].status == ERROR_RS)
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

    Server obj;

    while (true)
    {
        if (poll(obj.fds.data(), obj.fds.size(), -1) == -1)
        {
            std::cout << "poll error\n";
            continue;
        }
        for (size_t i = 0; i < obj.index_Cleint.size();)
        {
            

            pollfd &p = obj.fds[obj.index_Cleint[i]];

            if (p.fd == obj.server_fd && (p.revents & POLLIN))
            {
                obj.acceptNewCleint(p, i);
                continue;
            }
            if (p.revents & POLLIN)
            {
                if (!obj.receiveRequest(p, i))
                    continue;
            }
            if (p.revents & POLLOUT)
            {
                obj.iClnt = i;
                if (!obj.sendRespond(obj, p))
                {
                    continue;
                }
            }
            if ((p.revents & (POLLHUP | POLLERR)))
            {
                std::cout << "lagzouli 4444  \n\n";
                obj.close_client(p, i);
                continue;
            }
            i++;
        }
    }
    close(obj.server_fd);
}
