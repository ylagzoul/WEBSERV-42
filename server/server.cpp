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

Server::Server(std::vector<pollfd> &fds, int port, const std::string &IP)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cerr << "error in setsockopt\n";
    len = sizeof(addr);
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(IP.c_str());

    if (bind(server_fd, (sockaddr *)&addr, len) < 0)
    {
        close(server_fd);
        std::cout << "fail bind\n";
        exit(1);
    }
    if (listen(server_fd, 10) < 0)
    {
        close(server_fd);
        std::cout << "fail listen\n";
        exit(1);
    }

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0;

    fds.push_back(server_poll);
    index_Client.push_back(fds.size() - 1);
    data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());
    data[data.size() - 1].time = time(NULL);

    clientId = 0;
}

Webserv::Webserv()
{
    config_file();

    for (size_t i = 0; i < config.size(); i++)
    {
        Server obj(fds, config[i].listen_Port[0], config[i].listen_Ip[0]);
        server.push_back(obj);
        server[i].serverPort.push_back(obj);
        for (size_t t = 1; t < config[i].listen_Port.size(); t++)
        {
            Server objPort(fds, config[i].listen_Port[t], config[i].listen_Ip[t]);
            server[i].serverPort.push_back(objPort);
        }
    }
    // std::cout << "server.size()" << server.size() << std::endl;
}

void Server::close_client(Webserv &Web, pollfd &p, int i)
{
    close(p.fd);
    Web.fds.erase(Web.fds.begin() + index_Client[i]);

    req[i].clean();
    req.erase(req.begin() + i); // iteration problem
    recs[i].clean();
    recs.erase(recs.begin() + i);

    data.erase(data.begin() + i);
    index_Client.erase(index_Client.begin() + index_Client.size() - 1);
}

void Server::acceptNewCleint(Webserv &Web, pollfd &p, size_t &i)
{
    int client_fd = accept(server_fd, (sockaddr *)&addr, &len);
    if (client_fd != -1)
    {
        pollfd client_poll;
        client_poll.fd = client_fd;
        client_poll.events = POLLIN;
        client_poll.revents = 0;
        Web.fds.push_back(client_poll);
        index_Client.push_back(Web.fds.size() - 1);
        req.push_back(Request());
        recs.push_back(Record());
        data.push_back(Data());
        data[data.size() - 1].time = time(NULL);
    }
    else if (client_fd < 0)
    {
        std::cout << "close in accept\n"; // is it the right behavior ???
        close_client(Web, p, i);
        return;
    }
    i++;
}

bool Server::receiveRequest(Webserv &Web, pollfd &p, size_t &i, SessionsList &sList)
{
    char buffer[SIZESR] = {0};
    int bytes = recv(p.fd, buffer, SIZESR, 0);

    if (bytes > 0)
    {
        clientId = i;

        readRequest(req, clientId, buffer, bytes, Web.config[Web.s], sList);
        if (req[clientId].clientState == CLIENT_ON)
            p.events |= POLLOUT;
        return (true);
    }
    else if (bytes == 0)
    {
        close_client(Web, p, i);
        return (false);
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        else
        {
            close_client(Web, p, i);
            return (false);
        }
    }
}

bool Server::sendRespond(Webserv &Web, Server &obj, size_t &i, pollfd &p)
{
    if (data[i].sendNext)
    {
        Web.indexC = i;
        data[i].response = respond(Web, obj, recs[i], Web.config[Web.s], req[i]);
        if (data[i].response.empty() && (recs[i].status == ACTIVE_RS || recs[i].status == CGI_ACTIVE))
            return (true);
    }
    int sent = send(p.fd, data[i].response.c_str(), data[i].response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    recs[i].number += sent;
    if (sent > 0)
    {
        data[i].response = data[i].response.substr(sent);
        data[i].sendNext = data[i].response.empty();
    }
    if (sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        else
        {
            close_client(Web, p, i);
            return (false);
        }
    }
    if ((data[i].sendNext && recs[i].status == CLOSED_RS) || recs[i].status == ERROR_RS)
    {
        p.events &= ~POLLOUT;

        if (recs[i].clientState == DISCONNECTED)
        {
            close_client(Web, p, i);
            return (false);
        }
        else
        {
            req[i] = Request();
            recs[i] = Record();
            data[i].time = time(NULL);
        }
    }
    return (true);
}

int main(/*int arc,char *arv[]*/)
{

    Webserv Web;
    SessionsList sList;

    while (true)
    {
        if (poll(Web.fds.data(), Web.fds.size(), -1) == -1) // MOCHKIL F TIMEOUT!!!!! 3LA 7SAB HANDLE DISCONNECTION DYAL CLIENT
        {
            std::cout << "poll error\n";
            continue;
        }
        for (Web.s = 0; Web.s < Web.server.size(); Web.s++)
        {
            for (size_t t = 0; t < Web.server[Web.s].serverPort.size(); t++)
            {
                for (size_t i = 0; i < Web.server[Web.s].serverPort[t].index_Client.size();)
                {
                    if (sList.checkIfNotNull())
                        sList.checkLimitSessions();
                    pollfd &p = Web.fds[Web.server[Web.s].serverPort[t].index_Client[i]];

                    if (p.fd == Web.server[Web.s].serverPort[t].server_fd && (p.revents & POLLIN)) // CHECK ALL SERVER FDS
                    {
                        Web.server[Web.s].serverPort[t].acceptNewCleint(Web, p, i);
                        continue;
                    }
                    if (p.revents & (POLLIN))
                    {
                        if (!Web.server[Web.s].serverPort[t].receiveRequest(Web, p, i, sList))
                            continue;
                    }
                    if ((p.revents & POLLOUT))
                    {

                        Web.server[Web.s].serverPort[t].iClnt = i;
                        if (!Web.server[Web.s].serverPort[t].sendRespond(Web, Web.server[Web.s].serverPort[t], i, p))
                            continue;
                    }
                    // if (/*(i != 0 && time(NULL) - obj.data[i].time > 1000) || */(p.revents & (POLLERR)))
                    // {
                    //     std::cout << "i = " << i << std::endl;
                    //     std::cout << "close clneit about finsh time..............................................\n\n";
                    //     obj.close_client(p, i);
                    //     std::cout << time(NULL) - obj.data[i].time << std::endl;
                    //     continue;
                    // }
                    i++;
                }
            }
        }
    }
    // close(obj.server_fd);
}
