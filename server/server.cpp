#include "server.hpp"
#include <iostream>
int bvc;

Data::Data()
{
    rspEnd = true;
    maxSend = 0;
    sendNext = true;
    reqEnd = "\r\n\r\n";
    recvSize = 0;
    recvNum = 0;
    time = ::time(NULL);
}

Server::Server(Webserv &Web, int port, const std::string &IP)
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
    if (listen(server_fd, 400) < 0)
    {
        close(server_fd);
        std::cout << "fail listen\n";
        exit(1);
    }

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0;

    Web.fds.push_back(server_poll);
    Web.typeFd[Web.fds.size() - 1] = "server";
    Web.statusFd[Web.fds.size() - 1] = "active";
    index_Client.push_back(Web.fds.size() - 1);
    Web.data.push_back(Data());
    Web.recs.push_back(Record());
    Web.req.push_back(Request());
    Web.data[Web.data.size() - 1].time = time(NULL);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
}

Webserv::Webserv(std::string nameFielConfig)
{
    config_file(nameFielConfig);

    for (size_t i = 0; i < config.size(); i++)
    {
        Server obj(*this, config[i].listen_Port[0], config[i].listen_Ip[0]);
        server.push_back(obj);
        server[i].serverPort.push_back(obj);
        for (size_t t = 1; t < config[i].listen_Port.size(); t++)
        {
            Server objPort(*this, config[i].listen_Port[t], config[i].listen_Ip[t]);
            server[i].serverPort.push_back(objPort);
        }
    }
}

void Server::close_client(Webserv &Web, size_t &indexClnt)
{
    if (Web.recs[indexClnt].status == CGI_ACTIVE)
        closeCGI(Web, Web.recs[indexClnt].cgiData, Web.req[indexClnt].method, indexClnt);
    close(Web.fds[indexClnt].fd);
    Web.fds[indexClnt].fd = -1;
    // Web.fds[indexClnt].events = 0;
    // Web.fds[indexClnt].revents = 0;
    Web.statusFd[indexClnt] = "close";
    // index_Client.erase(index_Client.begin() + ii);
    Web.reuseFd.push_back(indexClnt);
    // Web.req[indexClnt].clean();
    Web.recs[indexClnt].clean();
}

void Server::acceptNewCleint(Webserv &Web)
{

    while (true)
    {
        int client_fd = accept(server_fd, (sockaddr *)&addr, &len);

        if (client_fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("accept");
            break;
        }
        pollfd client_poll;
        client_poll.fd = client_fd;
        client_poll.events = POLLIN;
        client_poll.revents = 0;
        if (Web.reuseFd.size() == 0)
        {
            cout << "new\n";
            Web.fds.push_back(client_poll);
            Web.typeFd[Web.fds.size() - 1] = "client";
            Web.statusFd[Web.fds.size() - 1] = "active";
            index_Client.push_back(Web.fds.size() - 1);
            Web.req.push_back(Request());
            Web.recs.push_back(Record());
            Web.data.push_back(Data());
            Web.data[Web.data.size() - 1].time = ::time(NULL);
        }
        else
        {
            cout << "reuseFd\n";

            Web.fds[Web.reuseFd[0]] = client_poll;
            Web.typeFd[Web.reuseFd[0]] = "client";
            Web.statusFd[Web.reuseFd[0]] = "active";
            // index_Client.push_back(Web.reuseFd[0]);
            Web.req[Web.reuseFd[0]] = Request();
            Web.recs[Web.reuseFd[0]] = Record();
            Web.data[Web.reuseFd[0]] = Data();
            Web.reuseFd.erase(Web.reuseFd.begin() + 0);
            Web.data[Web.reuseFd[0]].time = ::time(NULL);
        }
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }

}

bool Server::receiveRequest(Webserv &Web, size_t &indexClnt, SessionsList &sList)
{

    char buffer[SIZESR] = {0};
    int bytes = recv(Web.fds[indexClnt].fd, buffer, SIZESR, 0);
    if (bytes > 0)
    {
        Web.data[indexClnt].time = ::time(NULL);
        readRequest(Web.req[indexClnt], buffer, bytes, Web.config[Web.s], sList, Web.recs[indexClnt].data);
        if (Web.req[indexClnt].clientState == CLIENT_ON)
            Web.fds[indexClnt].events |= POLLOUT;
        return (true);
    }
    else
    {
        close_client(Web, indexClnt);
        return (false);
    }
}

bool Server::sendRespond(Webserv &Web, size_t &indexClnt)
{
    if (Web.data[indexClnt].sendNext)
    {
        Web.indexC = indexClnt;
        Web.data[indexClnt].response = respond(Web, Web.recs[indexClnt], Web.config[Web.s], Web.req[indexClnt], indexClnt);

        if (Web.data[indexClnt].response.empty() && (Web.recs[indexClnt].status == ACTIVE_RS || Web.recs[indexClnt].status == CGI_ACTIVE))
            return (true);
    }
    int sent = send(Web.fds[indexClnt].fd, Web.data[indexClnt].response.c_str(), Web.data[indexClnt].response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    Web.recs[indexClnt].number += sent;
    if (sent > 0)
    {
        Web.data[indexClnt].response = Web.data[indexClnt].response.substr(sent);
        Web.data[indexClnt].sendNext = Web.data[indexClnt].response.empty();
    }
    if (sent < 0)
    {
        close_client(Web, indexClnt);
        return (false);
    }
    if ((Web.data[indexClnt].sendNext && Web.recs[indexClnt].status == CLOSED_RS) || Web.recs[indexClnt].status == ERROR_RS)
    {
        Web.fds[indexClnt].events &= ~POLLOUT;

        if (Web.recs[indexClnt].clientState == DISCONNECTED)
        {
            close_client(Web, indexClnt);
            return (false);
        }
        else
        {
            Web.recs[indexClnt].clean();
            Web.req[indexClnt] = Request();
            Web.recs[indexClnt] = Record();
            Web.data[indexClnt] = Data();
            Web.data[indexClnt].time = ::time(NULL);
        }
    }
    return (true);
}

int main(int arc,char *arv[])
{
    if (arc != 2)
    {
        cout << "error arc \n";
        exit(1);
    }
    Webserv Web(arv[1]);

    SessionsList sList;
    try
    {
        while (true)
        {
            if (poll(&Web.fds[0], Web.fds.size(), 30000) == -1) // MOCHKIL F TIMEOUT!!!!! 3LA 7SAB HANDLE DISCONNECTION DYAL CLIENT
            {
                std::cout << "poll error\n";
                continue;
            }
            cout << "activ\n";
            int status;
            while ((waitpid(-1, &status, WNOHANG)) > 0)
                ;
            for (Web.s = 0; Web.s < Web.server.size(); Web.s++)
            {
                for (size_t t = 0;  t < Web.server[Web.s].serverPort.size(); t++)
                {
                    for (size_t i = 0; i < Web.server[Web.s].serverPort[t].index_Client.size(); i++)
                    {
                        // cout << "index_Client = " << Web.fds.size() << "---" << i<< " ----- " << Web.server[Web.s].serverPort[t].index_Client.size() << "\n";
                        Web.server[Web.s].serverPort[t].ii = i;
                        size_t indexClnt = Web.server[Web.s].serverPort[t].index_Client[i];

                        if (Web.statusFd[indexClnt] == "close")
                            continue;
                        if (sList.checkIfNotNull())
                            sList.checkLimitSessions();
    
    
                        if ((Web.typeFd[indexClnt] == "client" && time(NULL) - Web.data[indexClnt].time > 60) || (Web.fds[indexClnt].revents & (POLLERR | POLLHUP | POLLRDHUP)))
                        {
                            Web.server[Web.s].serverPort[t].close_client(Web, indexClnt);
                            continue;
                        }

                        if (Web.typeFd[indexClnt] == "server" && (Web.fds[indexClnt].revents & POLLIN)) // CHECK ALL SERVER FDS
                        {
                            Web.server[Web.s].serverPort[t].acceptNewCleint(Web);
                        }
                        else if (Web.typeFd[indexClnt] == "client" && (Web.fds[indexClnt].revents & POLLIN))
                        {
                            Web.server[Web.s].serverPort[t].receiveRequest(Web, indexClnt, sList);
                        }
                        else if (Web.typeFd[indexClnt] == "client" && (Web.fds[indexClnt].revents & POLLOUT))
                        {
                            Web.server[Web.s].serverPort[t].sendRespond(Web, indexClnt);
                        }
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
}
