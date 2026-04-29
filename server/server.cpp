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
    std::cout << "hello acceptNewCleint\n";
    int client_fd = accept(server_fd, (sockaddr *)&addr, &len);
    if (client_fd != -1)
    {
        std::cout << "accept Cleint number = " << i << "\n";
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
        std::cout << "close in accept\n";
        close(p.fd);
        fds.erase(fds.begin() + i);
        return ;
    }
    i++;
}

bool Server::receiveRequest(pollfd &p, size_t &i)
{
    char buffer[1024] = {0};
    int bytes = recv(p.fd, buffer, 1024, 0);
    std::cout << "buffer = " << buffer << "\n\n";
    if (bytes > 0)
    {
        clientId = i;
        readRequest(req, clientId, buffer, bytes, conf);
        std::cout << "req[i].status = " << req[i].status << "\n";
        // if (req[clientId].method == "GET" && (req[clientId].clientState == CLIENT_ON || req[clientId].reqState == ONE_ST))
        if ((req[clientId].clientState == CLIENT_ON))
        {
            std::cout << "check POLLOUT \n";
            p.events |= POLLOUT;
        }
        return (true);
    }
    else if (bytes == 0)
    {
        std::cout << "buffer = " << buffer << "\n\n";
        std::cout << "close in recv why? because disconnect" << " bytes = " << bytes << "\n";
        close(p.fd);
        fds.erase(fds.begin() + i);
        return (false); 
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {

            
            std::cout << "No data available now. Please try again later.\n";
            return (true);
        }
        else
        {
            std::cout << "close in recv why? because erorr" << " bytes = " << bytes << "\n";
            close(p.fd);
            fds.erase(fds.begin() + i);
            return (false);
        }
    }
}


bool Server::sendRespond(pollfd &p, size_t &i)
{
    if (data[i].sendNext)
    {
        std::cout << "path = " << req[i].path <<"\n";
        data[i].response = respond(recs[i], conf, req[i]);
        std::cout << "req[i].status = "<< req[i].status << " n" ;
    }
    int sent = send(p.fd, data[i].response.c_str(), data[i].response.size(), 0);

    // std::cout << "\n\nres " << data[i].response << "\n\n"; 
    std::cout << "send = " << sent << "size respond = " << data[i].response.size() << "\n";
    std::cout << "status " << recs[i].status << "\n"; 

    if (sent > 0)
    {
        data[i].response = data[i].response.substr(sent);
        data[i].sendNext = data[i].response.empty();
    }
    if (sent <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cout << "No data available now. Please try again later.\n";
            return (true);
        }
        else
        {
            std::cout << "close in send why? because erorr" << " sent = " << sent << "\n";
            std::cout << "close in sednd\n";
            close(p.fd);
            fds.erase(fds.begin() + i);
            return (false);
        }
    }
    if ((data[i].sendNext && recs[i].status == CLOSED_RS) || recs[i].status == ERROR_RS)
    {

        p.events &= ~POLLOUT;
        req[i] = Request();
        recs[i] = Record();
    }
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
                std::cout << "\n\naccept new cleint\n";
                obj.acceptNewCleint(p, i);
                std::cout << "end cleint\n\n";
                continue ;
            }
            if (p.revents & POLLIN)
            {
                std::cout << "\n\nstart receiveRequest\n";
                if (!obj.receiveRequest(p, i))
                    continue;

                std::cout << "path = " << obj.req[i].path << "number : " << obj.req[i].status << "\n"; 
                std::cout << "end sevdrespond\n\n";

            }
            if (p.revents & POLLOUT)
            {
                std::cout << "\n\nstart sevdrespond\n";
                if (!obj.sendRespond(p, i))
                {
                    continue ;
                }
                std::cout << "end sevdrespond\n\n";
            }
            if ((p.revents & (POLLHUP | POLLERR)))
            {
                std::cout << "\n\nclose fd\n\n";
                close(p.fd);
                obj.fds.erase(obj.fds.begin() + i);
                continue;
            }
            std::cout << "\n\nloop for   ++++++++++++++++++++++++\n\n";
            i++;
        }
        std::cout <<"\n\nloop while  ..............\n\n";
    }
    close(obj.server_fd);
}


// --------------------------------response main-----------------------------------------

// Request jibDikRequest()
// {
//     Request req;
//     req.status = 0;
//     req.method = "POST";
//     req.path = "/response/testfiles/file.txt";
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
//     Server server;
//     conf.uploadPath = "/heeloooooooooooooooooooo";
//     std::vector<Request> req(1, jibDikRequest());
//     // req[1].bodyPath = "/home/abouhki/goinfre/tmp/client2";
//     // req[2].bodyPath = "/home/abouhki/goinfre/tmp/client3";

//     // req[1].path = "/response/testfiles/client_2.";
//     // req[2].path = "/response/testfiles/client_3.json";
//     std::vector<Record> logs;
//     logs.push_back(Record());
//     logs.push_back(Record());
//     logs.push_back(Record());
//     std::ofstream of("print1");
//     // std::ofstream of("print2");
//     // std::ofstream of("print3");
//     std::string res;
//     res = respond(logs[0], server, conf, req[0]);
//     std::cout  << res << std::endl;
//     of << res << std::endl;
// }
