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
    clientId = 0;
}

int main()
{
    Config conf;
    Server obj;

    std::vector<Record> recs;
    std::vector<Request> req;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    conf.config_file();

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3051);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, len);
    listen(server_fd, 10);

    std::vector<pollfd> fds;

    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    fds.push_back(server_poll);

    obj.data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);

        if (ret == -1)
        {
            std::cout << "poll error\n";
            continue;
        }

        for (size_t i = 0; i < fds.size(); )
        {
            pollfd &p = fds[i];

            if (p.revents == 0)
            {
                i++;
                continue;
            }

            if (p.fd == server_fd && (p.revents & POLLIN))
            {
                int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
                if (client_fd != -1)
                {
                    pollfd client_poll;
                    client_poll.fd = client_fd;
                    client_poll.events = POLLIN;
                    fds.push_back(client_poll);

                    recs.push_back(Record());
                    req.push_back(Request());
                    obj.data.push_back(Data());
                }
                else if (client_fd < 0)
                {
                    close(p.fd);
                    fds.erase(fds.begin() + i);
                    continue;
                }

                p.revents = 0;
                i++;
                continue;
            }

            if (p.revents & POLLIN)
            {
                char buffer[1024] = {0};
                int bytes = recv(p.fd, buffer, 1024, 0);

                if (bytes > 0)
                {
                    int index = i - 1;
                    obj.clientId = i;
                    readRequest(req, index, buffer, bytes, conf);
                    

                    // بعد القراءة نسمح بالكتابة
                    p.events |= POLLOUT;
                }
                else
                {
                    close(p.fd);
                    fds.erase(fds.begin() + i);
                    continue;
                }
            }

            if (p.revents & POLLOUT)
            {
                int index = i - 1;

                if (obj.data[i].sendNext)
                {
                    obj.clientId = i;
                    obj.data[i].response =
                        respond(recs[index], obj, conf, req[index]);
                    std::cout << obj.data[i].response << "\n\n";
                    // exit(1);
                }

                int sent = send(p.fd,
                                obj.data[i].response.c_str(),
                                obj.data[i].response.size(),
                                0);

                if (sent > 0)
                {
                    obj.data[i].response = obj.data[i].response.substr(sent);
                    obj.data[i].sendNext = obj.data[i].response.empty();
                }
                if (sent < 0)
                {
                    close(p.fd);
                    fds.erase(fds.begin() + i);
                    continue;
                }
            }

            if ((p.revents & (POLLHUP | POLLERR)))
            {
                close(p.fd);
                fds.erase(fds.begin() + i);
                continue;
            }

            p.revents = 0;
            i++;
        }
    }

    close(server_fd);
}


// response main


// int main()
// {
//     Config conf;
//     Server obj;
//     std::vector<Record> recs;
//     recs.push_back(Record());
//     std::vector<Request> req;
    
//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     int n =0;

//     sockaddr_in addr;
//     socklen_t len = sizeof(addr);
//     conf.config_file();

//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(3001);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(server_fd, (sockaddr*)&addr, len);
//     listen(server_fd, 5);

//     std::vector<pollfd> fds;

//     pollfd server_poll;
//     server_poll.fd = server_fd;
//     server_poll.events = POLLIN;
//     fds.push_back(server_poll);

//     Data dt;
//     obj.data.push_back(dt);

//     while (true)
//     {
//         poll(fds.data(),fds.size(),-1);
//         if (obj.clientId == fds.size())
//             obj.clientId = 0;
//         if (fds[obj.clientId].revents & (POLLIN | POLLOUT | POLLHUP))
//         {
//             if (fds[obj.clientId].fd == server_fd)
//             {
//                 int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
//                 pollfd client_poll;
//                 client_poll.fd = client_fd;
//                 client_poll.events = POLLIN;
//                 fds.push_back(client_poll);
//                 recs.push_back(Record());
//                 req.push_back(Request());
//                 obj.data.push_back(dt);
//             }
//             else
//             {
//                 if (fds[obj.clientId].revents & POLLIN)
//                 {
//                     char buffer[1024] = {0};
//                     int bytes = recv(fds[obj.clientId].fd, buffer, 1024, 0);
//                     if (bytes > 0)
//                     {
//                         int index1 = obj.clientId - 1;
//                         readRequest(req, index1, buffer, bytes);
//                         fds[obj.clientId].events = POLLOUT;
//                     }
//                     // else if (bytes == 0)
//                     //     fds[obj.clientId].events = POLLHUP;
//                     // else
//                     //     fds[obj.clientId].revents = POLLERR;

//                 }
//                 if ((fds[obj.clientId].revents & POLLOUT))
//                 {
//                     int index = obj.clientId - 1;
//                     if (obj.data[obj.clientId].sendNext == true)
//                         obj.data[obj.clientId].response = respond(recs[index], obj, conf,req[index]);
//                     std::cout <<  obj.data[obj.clientId].response << std::endl;
//                     n += send(fds[obj.clientId].fd, obj.data[obj.clientId].response.c_str(), obj.data[obj.clientId].response.size(), 0);
//                     if (n > 0)
//                     {
//                         int max = obj.data[obj.clientId].response.size();
//                         if (n < max)
//                         {
//                             obj.data[obj.clientId].response = obj.data[obj.clientId].response.substr(n);
//                             obj.data[obj.clientId].sendNext = false;
//                         }
//                         else
//                             obj.data[obj.clientId].sendNext = true;
//                     }
//                     else if (n == -1)
//                     {
//                         // fds[obj.clientId].revents = POLLERR;
//                         obj.data[obj.clientId].sendNext = true;
//                     }
//                     n = 0;
//                 }
//                 else
//                 {
//                     close(fds[obj.clientId].fd); 
//                     fds.erase(fds.begin() + obj.clientId);
//                     std::cout << "The connection was cut off";
//                 }
//             }
//         }
//         else if (fds[obj.clientId].revents & POLLERR)
//         {
//             close(fds[obj.clientId].fd);
//             std::cout << "Socket error\n";
//         }
//         obj.clientId++;
//     }
//     close(server_fd);
// }


// using namespace std;

// int main()
// {
//     vector<Request> req;
//     Config con;
//     con.client_max_body_size = 14;
//     for (size_t i = 0; i < 3; i++)
//         req.push_back(Request());
//     int i = 0;
//         char buffer[] = "POST /paj//home HTTP/1.0\r\n
// Content-Length: 2\r\n
// \r\n\r\nkd";

//     string str = buffer;

//     readRequest(req, i, buffer, str.length(), con);
//     // readRequest(req, i, buffer, str.length(), con);
        

//     cout << req[0].path << endl;
//     cout << req[0].status << endl;
//     cout << req[0].method << endl;
//     cout << req[0].clientState << endl;


// }

// int main()
// {
//     Config conf;
//     conf.config_file();
//     Server server;
//     conf.uploadPath = "upload";
//     Request req;
//     req.status = 0;
//     req.method = "GET";
//     req.path = "/response/testfiles/kile";
//     req.httpVersion = "HTTP/1.0"; 
//     req.headers["Host"] = "localhost";
//     req.headers["Connection"] = "Keep-Alive";

//     std::vector<Record> logs;
//     logs.push_back(Record());

//     std::string res = respond(logs[0], server, conf, req);
//     std::cout  << res << std::endl;
// }