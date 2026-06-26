#include "server.hpp"
#include <iostream>
int bvc;
static int y;
Data::Data()
{
    rspEnd = true;
    maxSend = 0;
    sendNext = true;
    reqEnd = "\r\n\r\n";
    recvSize = 0;
    recvNum = 0;
}

Server::Server(std::vector<pollfd> &fds, int port, const std::string &ip)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    y++;
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cerr << "error in setsockopt\n";
    len = sizeof(addr);
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(server_fd, (sockaddr *)&addr, len) < 0)
    {
        close(server_fd);
        std::cout << "fail bind\n";
        exit(1);
    }
    if (listen(server_fd, 50) < 0)
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
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

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
}

void Server::close_client(Webserv &Web, pollfd &p, int i)
{
    // std::cout << "cose client    ,,,,\n";
    close(p.fd);
    (void)p;
    Web.fds.erase(Web.fds.begin() + index_Client[i]);

    req[i].clean();
    req.erase(req.begin() + i); // iteration problem
    recs[i].clean();
    recs.erase(recs.begin() + i);

    data.erase(data.begin() + i);
    index_Client.erase(index_Client.begin() + index_Client.size() - 1);// fds.fd -1 --> fd==5-index=3  fd==5-index=2
}

void Server::acceptNewCleint(Webserv &Web, pollfd &p, size_t &i)
{
    (void)p;
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
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }
    else if (client_fd < 0)
    {
        std::cout << "error in accept\n";
        // close_client(Web, p, i);
        return ;
    }
    i++;
}

bool Server::receiveRequest(Webserv &Web, pollfd &p, size_t &i, SessionsList &sList)
{

    char buffer[SIZESR] = {0};
    int bytes = recv(p.fd, buffer, SIZESR, 0);
    if (bytes > 0)
    {
        // cout << "start receiveRequest";
        clientId = i;
        // cout << "ENTERED\n";
        readRequest(req[clientId], buffer, bytes, Web.config[Web.s], sList, recs[clientId].data);// clientid
        // std::cout << "request: " << buffer << "status: " << req[clientId].status << std::endl;
        // cout << "EXITED\n";
        // std::cout << req[clientId].clientState << std::endl;
        if (req[clientId].clientState == CLIENT_ON)
            p.events |= POLLOUT;
        return (true);
    }
    else if (bytes == 0)
    {
        // cout << "close bytes == 0\n";
        close_client(Web, p, i);
        return (false);
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        else
        {
            // cout << "close bytes == -1\n";
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
        // cout << "start respond\n";
        data[i].response = respond(Web, obj, recs[i], Web.config[Web.s], req[i]);
        // cout << "end respond\n";
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
            cout << "close sent == -1\n";
            close_client(Web, p, i);
            return (false);
        }
    }
    if ((data[i].sendNext && recs[i].status == CLOSED_RS) || recs[i].status == ERROR_RS)
    {
        p.events &= ~POLLOUT;
        
        if (recs[i].clientState == DISCONNECTED)
        {
            // cout << "close clientState == DISCONNECTED\n";
            close_client(Web, p, i);
            return (false);
            
        }
        else
        {
            recs[i].clean();
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
    try
    {
        while (true)
        {
           
            // cout << "while true\n";
            if (poll(&Web.fds[0], Web.fds.size(), 30) == -1) // MOCHKIL F TIMEOUT!!!!! 3LA 7SAB HANDLE DISCONNECTION DYAL CLIENT
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
                        // cout << "for  for\n";
                        if (sList.checkIfNotNull())
                        sList.checkLimitSessions();
                        // cout << "1111111111111111111111\n";
                        pollfd &p = Web.fds[Web.server[Web.s].serverPort[t].index_Client[i]];
                        
                        if (p.fd == Web.server[Web.s].serverPort[t].server_fd && (p.revents & POLLIN)) // CHECK ALL SERVER FDS
                        {
                            // cout << "acceptNewCleint\n";
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
                                continue ;
                        }
                        if ((i != 0 && time(NULL) - Web.server[Web.s].serverPort[t].data[i].time > 30) || (p.revents & (POLLERR)))
                        {
                            if (p.revents & (POLLERR))
                                cout << "close POLLERR\n";
                            else
                                std::cout << "close client about finsh time..............................................\n\n";
                            Web.server[Web.s].serverPort[t].close_client(Web, p, i);
                            continue;
                        }
                        i++;
                    }
                }
            }
        }

    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        for (size_t s = 0; s < Web.server.size(); s++)
        {
            for (size_t t = 0; t < Web.server[s].serverPort.size(); t++)
            {
                std::cout << "close server     ...\n";
                close(Web.fds[Web.server[s].serverPort[t].index_Client[0]].fd);
                cout << t << "kdfk " << s << std::endl;
            }
        }
        return 1;
    }

    // for (size_t s = 0; s < Web.server.size(); s++)
    // {
    //     for (size_t t = 0; t < Web.server[s].serverPort.size(); t++)
    //     {
    //         std::cout << "close server     ...\n";
    //         close(Web.fds[Web.server[s].serverPort[t].index_Client[0]].fd);
    //         cout << t << "kdfk " << s << std::endl;
    //     }
    // }
}
