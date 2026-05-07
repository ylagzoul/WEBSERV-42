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
    if (access(req[i].bodyFile.c_str(), F_OK) != -1)
        std::remove(req[i].bodyPath.c_str());
    fds.erase(fds.begin() + i);
    req.erase(req.begin() + i);//iteration problem 
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


bool Server::receiveRequest(pollfd &p, size_t &i)
{
    char buffer[1024] = {0};
    int bytes = recv(p.fd, buffer, 1024, 0);
    if (bytes > 0)
    {
        clientId = i;
        cout << "Mohammed 1\n";
        readRequest(req, clientId, buffer, bytes, conf);
        // cout << "-->" << req[i].path << std::endl;
        cout << "Mohammed 2\n";
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
        
        data[i].response = respond(recs[i].data, recs[i], conf, req[i]);
        if (data[i].response.empty() && recs[i].status == ACTIVE_RS)
            return (true);
        
    }
    int sent = send(p.fd, data[i].response.c_str(), data[i].response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    recs[i].number+=sent;
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
        close_client(p, i);

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
        for (size_t i = 0; i < obj.fds.size();)
        {

            pollfd &p = obj.fds[i];

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

// // --------------------------------response main-----------------------------------------

// Request jibDikRequest()
// {
//     Request req;
//     req.status = 0;
//     req.method = "GET";
//     req.path = "//hh/h/h//////hhg///gh///gh/////ghg////gh////gh";
//     req.bodyPath = "/home/abouhki/goinfre/tmp/client1";
//     req.httpVersion = "HTTP/1.0";
//     req.headers["Host"] = "localhost";
//     req.headers["Connection"] = "Keep-Alive";
//     req.headers["content-type"] = "application/json";
//     return req;
// }

// int main()
// {
//     Config conf;
//     conf.config_file();
//     // Server server;
//     conf.uploadPath = "/hello";
//     std::vector<Request> req(1, jibDikRequest());
//     // req[1].bodyPath = "/home/abouhki/goinfre/tmp/client2";
//     // req[2].bodyPath = "/home/abouhki/goinfre/tmp/client3";

//     // req[1].path = "/response/testfiles/client_2.";
//     // req[2].path = "/response/testfiles/client_3.json";
//     std::vector<Record> logs;
//     logs.push_back(Record());
//     logs.push_back(Record());
//     logs.push_back(Record());
//     std::ofstream of("print1.html");
//     // std::ofstream of("print2");
//     // std::ofstream of("print3");
//     std::string res;
//     while(logs[0].status != ERROR_RS && logs[0].status != CLOSED_RS)
//     {
//         res = respond(logs[0], conf, req[0]);
//         of << res;
//     }
// }
