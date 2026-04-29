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

// int main()
// {
//     Config conf;
//     Server obj;

//     std::vector<Record> recs;
//     std::vector<Request> req;

//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr;
//     socklen_t len = sizeof(addr);

//     conf.config_file();

//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8031);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(server_fd, (sockaddr*)&addr, len);
//     listen(server_fd, 10);

//     std::vector<pollfd> fds;

//     pollfd server_poll;
//     server_poll.fd = server_fd;
//     server_poll.events = POLLIN;
//     fds.push_back(server_poll);

//     obj.data.push_back(Data());
//     recs.push_back(Record());
//     req.push_back(Request());

//     while (true)
//     {
//         int ret = poll(fds.data(), fds.size(), -1);

//         if (ret == -1)
//         {
//             std::cout << "poll error\n";
//             continue;
//         }

//         for (size_t i = 0; i < fds.size(); )
//         {
//             pollfd &p = fds[i];

//             if (p.revents == 0)
//             {
//                 i++;
//                 continue;
//             }

//             if (p.fd == server_fd && (p.revents & POLLIN))
//             {
//                 int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
//                 if (client_fd != -1)
//                 {
//                     pollfd client_poll;
//                     client_poll.fd = client_fd;
//                     client_poll.events = POLLIN;
//                     fds.push_back(client_poll);

//                     recs.push_back(Record());
//                     req.push_back(Request());
//                     obj.data.push_back(Data());
//                 }

//                 p.revents = 0;
//                 i++;
//                 continue;
//             }

//             if (p.revents & POLLIN)
//             {
//                 char buffer[1024] = {0};
//                 int bytes = recv(p.fd, buffer, 1024, 0);

//                 if (bytes > 0)
//                 {
//                     int index = i - 1;
//                     obj.clientId = i;
//                     readRequest(req, index, buffer, bytes);

//                     // بعد القراءة نسمح بالكتابة
//                     p.events |= POLLOUT;
//                 }
//                 else
//                 {
//                     close(p.fd);
//                     fds.erase(fds.begin() + i);
//                     continue;
//                 }
//             }

//             if (p.revents & POLLOUT)
//             {
//                 int index = i - 1;

//                 if (obj.data[i].sendNext)
//                 {
//                     obj.clientId = i;
//                     obj.data[i].response =
//                         respond(recs[index], obj, conf, req[index]);
//                 }

//                 int sent = send(p.fd,
//                                 obj.data[i].response.c_str(),
//                                 obj.data[i].response.size(),
//                                 0);

//                 if (sent > 0)
//                 {
//                     obj.data[i].response = obj.data[i].response.substr(sent);
//                     obj.data[i].sendNext = obj.data[i].response.empty();
//                 }
//             }

//             if (p.revents & (POLLHUP | POLLERR))
//             {
//                 close(p.fd);
//                 fds.erase(fds.begin() + i);
//                 continue;
//             }

//             p.revents = 0;
//             i++;
//         }
//     }

//     close(server_fd);
// }


// response main

Request jibDikRequest()
{
    Request req;
    req.status = 0;
    req.method = "POST";
    req.path = "/response/testfiles/file.txt";
    req.bodyPath = "/home/abouhki/goinfre/tmp/client1";
    req.httpVersion = "HTTP/1.0";
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";
    req.headers["content-type"] = "application/json";
    return req;
}

int main()
{
    Config conf;
    conf.config_file();
    Server server;
    conf.uploadPath = "/heeloooooooooooooooooooo";
    std::vector<Request> req(1, jibDikRequest());
    // req[1].bodyPath = "/home/abouhki/goinfre/tmp/client2";
    // req[2].bodyPath = "/home/abouhki/goinfre/tmp/client3";

    // req[1].path = "/response/testfiles/client_2.";
    // req[2].path = "/response/testfiles/client_3.json";
    std::vector<Record> logs;
    logs.push_back(Record());
    logs.push_back(Record());
    logs.push_back(Record());
    std::ofstream of("print1");
    // std::ofstream of("print2");
    // std::ofstream of("print3");
    std::string res;
    res = respond(logs[0], server, conf, req[0]);
    std::cout  << res << std::endl;
    of << res << std::endl;
    // sleep(3);
    // res = respond(logs[0], server, conf, req[0]);
    // std::cout  << res << std::endl;
    // of << res << std::endl;
    // sleep(3);
    // res = respond(logs[0], server, conf, req[0]);
    // std::cout  << res << std::endl;
    // of << res;
    // sleep(3);
    // res = respond(logs[0], server, conf, req[0]);
    // std::cout  << res << std::endl;
    // of << res;
    // res = respond(logs[0], server, conf, req[0]);
    // std::cout  << res << std::endl;
    // of << res;
    // sleep(3);

    // res = respond(logs[0], server, conf, req[0]);
    // std::cout  << res << std::endl;
    // of << res;
}