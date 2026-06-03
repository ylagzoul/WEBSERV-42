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
    index_Client.push_back(0);

    data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());
    data[data.size()-1].time = time(NULL);

    clientId = 0;
}

void Server::close_client(pollfd &p, int i)
{
    close(p.fd);
    fds.erase(fds.begin() + index_Client[i]);

    req[index_Client[i]].clean();
    req.erase(req.begin() + index_Client[i]); // iteration problem

    recs[index_Client[i]].clean();
    recs.erase(recs.begin() + index_Client[i]);

    data.erase(data.begin() + index_Client[i]);
    index_Client.erase(index_Client.begin() + index_Client.size() - 1);
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
        index_Client.push_back(fds.size() - 1);
        req.push_back(Request());
        recs.push_back(Record());
        data.push_back(Data());
        data[data.size()-1].time = time(NULL);
        std::cout << "data.size()-1 = " << data.size()-1<< std::endl; 

    }
    else if (client_fd < 0)
    {
        std::cout << "close in accept\n"; // is it the right behavior ???
        close_client(p, i);
        return;
    }
    i++;
}


bool Server::receiveRequest(pollfd &p, size_t &i, SessionsList &sList)
{
    char buffer[SIZESR] = {0};
    int bytes = recv(p.fd, buffer, SIZESR, 0);

    // std::cout << "buffer = " << buffer << std::endl;
    if (bytes > 0)
    {
        // cout << "test11\n\n";
        clientId = index_Client[i];
        // cout << "clientId =  "  << clientId << std::endl;
        readRequest(req, clientId, buffer, bytes, conf, sList);
        // cout << "--> " << req[i].clientState << std::endl;
        // cout << "--> " << req[i].status << std::endl;

        // cout << "test21\n\n";

        if (req[clientId].clientState == CLIENT_ON)
        {
            // cout << "pillout\n\n";
            p.events |= POLLOUT;
        }
        return (true);
    }
    else if (bytes == 0)
    {
        std::cout << "bytes 0\n"; 
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
    if (data[index_Client[iClnt]].sendNext)
    {
        data[index_Client[iClnt]].response = respond(obj, recs[index_Client[iClnt]].data, recs[index_Client[iClnt]], conf, req[index_Client[iClnt]]);
        // std::cout << obj.recs[index_Client[iClnt]].status << std::endl;
        if (data[index_Client[iClnt]].response.empty() && (recs[index_Client[iClnt]].status == ACTIVE_RS || recs[index_Client[iClnt]].status == CGI_ACTIVE))
            return (true);
    }
    // cout << "test33\n\n";
    int sent = send(p.fd, data[index_Client[iClnt]].response.c_str(), data[index_Client[iClnt]].response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    recs[index_Client[iClnt]].number += sent;
    if (sent > 0)
    {
        data[index_Client[iClnt]].response = data[index_Client[iClnt]].response.substr(sent);
        data[index_Client[iClnt]].sendNext = data[index_Client[iClnt]].response.empty();
    }
    if (sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return (true);
        }
        else
        {
            close_client(p, iClnt);
            return (false);
        }
    }
    if ((data[index_Client[iClnt]].sendNext && recs[index_Client[iClnt]].status == CLOSED_RS) || recs[index_Client[iClnt]].status == ERROR_RS)
    {
        p.events &= ~POLLOUT;

        if (recs[index_Client[iClnt]].clientState  == DISCONNECTED)
            close_client(p, iClnt);
        else
        {
            req[index_Client[iClnt]] = Request();
            recs[index_Client[iClnt]] = Record();
        }

        data[index_Client[iClnt]].time = time(NULL);
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
        for (size_t i = 0; i < obj.index_Client.size();)
        {

            pollfd &p = obj.fds[obj.index_Client[i]];

            if (p.fd == obj.server_fd && (p.revents & POLLIN))//CHECK ALL SERVER FDS
            {
                obj.acceptNewCleint(p, i);
                continue ;
            }
            if (p.revents & POLLIN)
            {
                if (!obj.receiveRequest(p, i, sList))
                    continue;
            }

            if ((p.revents & POLLOUT) || (obj.recs[obj.index_Client[i]].status == CGI_ACTIVE))
            {

                obj.iClnt = i;
                if (!obj.sendRespond(obj, p))
                {
                    continue;
                }
            }
            if ((i != 0 && time(NULL) - obj.data[i].time > 1000) || (p.revents & (POLLHUP | POLLERR))) 
            {
                std::cout << "i = " << i << std::endl;
                std::cout << "close clneit about finsh time..............................................\n\n";
                obj.close_client(p, i);
                continue;
            }
            i++;
        }
        
    }
    close(obj.server_fd);
}
