#include "server.hpp"

Server::Server(Webserv &Web, int port, const std::string &IP)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        for (size_t i = 0; i < Web.fds.size(); i++)
        {
            if (Web.statusFd[i] == "active")
                close(Web.fds[i].fd);
        }
        throw std::runtime_error("Error: fail socket");
    }
    int opt = -1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        for (size_t i = 0; i < Web.fds.size(); i++)
        {
            if (Web.statusFd[i] == "active")
                close(Web.fds[i].fd);
        }
        close(server_fd);
        throw std::runtime_error("Error: setsockopt() failed");
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(IP.c_str());
    len = sizeof(addr);
    if (bind(server_fd, (sockaddr *)&addr, len) < 0)
    {
        for (size_t i = 0; i < Web.fds.size(); i++)
        {
            if (Web.statusFd[i] == "active")
                close(Web.fds[i].fd);
        }
        close(server_fd);
        throw std::runtime_error("Error: bind() failed");
    }
    if (listen(server_fd, 400) < 0)
    {
        for (size_t i = 0; i < Web.fds.size(); i++)
        {
            if (Web.statusFd[i] == "active")
                close(Web.fds[i].fd);
        }
        close(server_fd);
        throw std::runtime_error("Error: listen() failed");
    }

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0;

    Web.fds.push_back(server_poll);
    Web.typeFd[Web.fds.size() - 1] = "server";
    Web.statusFd[Web.fds.size() - 1] = "active";
    index_Client.push_back(Web.fds.size() - 1);
    Web.recs.push_back(Record());
    Web.req.push_back(Request());
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
}

Webserv::Webserv(std::string configNameFile)
{
    config_file(configNameFile);

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
    if (Web.recs[indexClnt].status == CGI_ACTIVE_RS)
        closeCGI(Web, indexClnt);
    close(Web.fds[indexClnt].fd);
    Web.fds[indexClnt].fd = -1;
    Web.statusFd[indexClnt] = "close";
    Web.server[Web.s].serverPort[Web.t].reuseFd.push_back(indexClnt);
    Web.recs[indexClnt].clean();
}

void Server::acceptNewClient(Webserv &Web)
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
        return;
    pollfd client_poll;
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    client_poll.revents = 0;

    if (Web.server[Web.s].serverPort[Web.t].reuseFd.size() == 0)
    {
        Web.fds.push_back(client_poll);
        Web.typeFd[Web.fds.size() - 1] = "client";
        Web.statusFd[Web.fds.size() - 1] = "active";
        index_Client.push_back(Web.fds.size() - 1);
        Web.req.push_back(Request());
        Web.recs.push_back(Record());
        Web.recs[Web.recs.size() - 1].time = std::time(NULL);
    }
    else
    {
        Web.fds[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = client_poll;
        Web.typeFd[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = "client";
        Web.statusFd[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = "active";
        Web.req[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = Request();
        Web.recs[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = Record();
        Web.server[Web.s].serverPort[Web.t].reuseFd.erase(Web.server[Web.s].serverPort[Web.t].reuseFd.begin());
        Web.recs[Web.server[Web.s].serverPort[Web.t].reuseFd[0]].time = std::time(NULL);
    }
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
}

bool Server::receiveRequest(Webserv &Web, size_t &indexClnt, SessionsList &sList)
{
    char buffer[SIZESR];
    int bytes = recv(Web.fds[indexClnt].fd, buffer, SIZESR, 0);
    if (bytes > 0)
    {
        Web.recs[indexClnt].time = std::time(NULL);
        readRequest(Web.req[indexClnt], buffer, bytes, Web.config[Web.s], sList, Web.recs[indexClnt].data);

        if (Web.req[indexClnt].clientState == CLIENT_ON)
        {
            Web.fds[indexClnt].events &= ~POLLIN;
            Web.fds[indexClnt].events |= POLLOUT;
        }
        return (true);
    }
    else
        close_client(Web, indexClnt);
    return (false);
}

bool Server::sendRespond(Webserv &Web, size_t &indexClnt)
{
    if (Web.recs[indexClnt].sendNext)
    {

        Web.recs[indexClnt].response = respond(Web, Web.recs[indexClnt], Web.config[Web.s], Web.req[indexClnt], indexClnt);
        if (Web.recs[indexClnt].response.empty() && (Web.recs[indexClnt].status == ACTIVE_RS || Web.recs[indexClnt].status == CGI_ACTIVE_RS))
            return (true);
    }
    int sent = send(Web.fds[indexClnt].fd, Web.recs[indexClnt].response.c_str(), Web.recs[indexClnt].response.size(), MSG_NOSIGNAL);
    if (sent >= 0)
    {
        Web.recs[indexClnt].response = Web.recs[indexClnt].response.substr(sent);
        Web.recs[indexClnt].sendNext = Web.recs[indexClnt].response.empty();
    }
    if (sent < 0)
    {
        close_client(Web, indexClnt);
        return (false);
    }
    if ((Web.recs[indexClnt].sendNext && Web.recs[indexClnt].status == CLOSED_RS) || Web.recs[indexClnt].status == ERROR_RS)
    {
        Web.fds[indexClnt].events &= ~POLLOUT;
        Web.fds[indexClnt].events |= POLLIN;

        if (Web.recs[indexClnt].clientState == DISCONNECTED)
        {
            close_client(Web, indexClnt);
            return (false);
        }
        else
        {
            Web.recs[indexClnt].clean();
            Web.recs[indexClnt] = Record();
            Web.req[indexClnt] = Request();
            Web.recs[indexClnt].time = std::time(NULL);
        }
    }
    return (true);
}

int main(int ac, char *arv[])
{
    try
    {
        if (ac != 2)
            throw std::runtime_error("Error: you should enter the config file only");
        Webserv Web(arv[1]);
        SessionsList sList;
        while (true)
        {
            if (poll(&Web.fds[0], Web.fds.size(), 30000) == -1)
            {
                for (size_t i = 0; i < Web.fds.size(); i++)
                {
                    if (Web.statusFd[i] == "active")
                        close(Web.fds[i].fd);
                }
                throw std::runtime_error("Error: poll() failed");
            }
            for (Web.s = 0; Web.s < Web.server.size(); Web.s++)
            {
                for (Web.t = 0; Web.t < Web.server[Web.s].serverPort.size(); Web.t++)
                {
                    for (Web.i = 0; Web.i < Web.server[Web.s].serverPort[Web.t].index_Client.size(); Web.i++)
                    {
                        size_t indexClnt = Web.server[Web.s].serverPort[Web.t].index_Client[Web.i];

                        if (Web.statusFd[indexClnt] == "close" || Web.typeFd[indexClnt] == "pipe")
                            continue;

                        if (sList.checkIfNotNull())
                            sList.checkLimitSessions();

                        if ((Web.typeFd[indexClnt] == "client" && time(NULL) - Web.recs[indexClnt].time > TIMEOUT) || (Web.fds[indexClnt].revents & (POLLERR | POLLHUP)))
                        {
                            if (time(NULL) - Web.recs[indexClnt].time > TIMEOUT && Web.recs[indexClnt].status == CGI_ACTIVE_RS)
                            {
                                int status;
                                kill(Web.recs[indexClnt].cgiData.pid, SIGKILL);
                                int ret = waitpid(Web.recs[indexClnt].cgiData.pid, &status, 0);
                                if (ret > 0)
                                {
                                    Web.recs[indexClnt].pi = true;
                                    Web.recs[indexClnt].cgiData.status = 504;

                                    std::vector<int>::iterator it = std::find(Web.cgiPid.begin(), Web.cgiPid.end(), Web.recs[indexClnt].cgiData.pid);
                                    int index = 0;
                                    if (it != Web.cgiPid.end())
                                        index = it - Web.cgiPid.begin();
                                    Web.cgiPid.erase(Web.cgiPid.begin() + index);
                                    Web.indexPid.erase(Web.indexPid.begin() + index);
                                }
                            }
                            else
                            {
                                Web.server[Web.s].serverPort[Web.t].close_client(Web, indexClnt);
                                continue;
                            }
                        }

                        if (Web.typeFd[indexClnt] == "server" && (Web.fds[indexClnt].revents & POLLIN))
                        {
                            Web.server[Web.s].serverPort[Web.t].acceptNewClient(Web);
                        }
                        else if (Web.typeFd[indexClnt] == "client" && (Web.fds[indexClnt].revents & POLLIN))
                        {
                            Web.server[Web.s].serverPort[Web.t].receiveRequest(Web, indexClnt, sList);
                        }
                        else if (Web.typeFd[indexClnt] == "client" && (Web.fds[indexClnt].revents & POLLOUT))
                        {
                            Web.server[Web.s].serverPort[Web.t].sendRespond(Web, indexClnt);
                        }
                    }
                    for (size_t i = 0; i < Web.cgiPid.size();)
                    {
                        int status;
                        int ret = waitpid(Web.cgiPid[i], &status, WNOHANG);
                        if (ret > 0)
                        {
                            Web.recs[Web.indexPid[i]].pi = true;
                            if (WIFEXITED(status))
                            {
                                if (WEXITSTATUS(status) > 0)
                                    Web.recs[Web.indexPid[i]].cgiData.status = 500;
                            }
                            else if (WIFSIGNALED(status))
                                Web.recs[Web.indexPid[i]].cgiData.status = 500;

                            Web.cgiPid.erase(Web.cgiPid.begin() + i);
                            Web.indexPid.erase(Web.indexPid.begin() + i);
                        }
                        else
                            i++;
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
