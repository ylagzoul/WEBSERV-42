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
    i = 0;
}

// int main()
// {
//     Config conf;
//     Server obj;
    
//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     int n;

//     sockaddr_in addr;
//     socklen_t len = sizeof(addr);

//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(452);
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
//         if (obj.i == fds.size())
//             obj.i = 0;
//         if (fds[obj.i].revents & (POLLIN | POLLOUT | POLLHUP))
//         {
//             if (fds[obj.i].fd == server_fd)
//             {
//                 int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
//                 pollfd client_poll;
//                 client_poll.fd = client_fd;
//                 client_poll.events = POLLIN;
//                 fds.push_back(client_poll);
//                 obj.data.push_back(dt);
//             }
//             else
//             {
//                 if (fds[obj.i].revents & POLLIN)
//                 {
//                     char buffer[1024] = {0};
//                     int bytes = recv(fds[obj.i].fd, buffer, 1024, 0);
//                     if (bytes > 0)
//                     {
//                         // function_mradouan(cpyBuffer);
//                     }
//                     else if (bytes == 0)
//                         fds[obj.i].events = POLLHUP;
//                     else
//                         fds[obj.i].revents = POLLERR;

//                     std::cout << "Request:\n" << buffer << std::endl;
//                 }
//                 else if ((fds[obj.i].revents & POLLOUT) && (obj.data[obj.i].rspEnd))
//                 {
//                     if (obj.data[obj.i].sendNext == true)
//                         obj.data[obj.i].response = respond(obj, conf);
//                     n += send(fds[obj.i].fd, obj.data[obj.i].response.substr(n).c_str(), obj.data[obj.i].response.size(), 0);
//                     if (n > 0)
//                     {
//                         if (n != obj.data[obj.i].maxSend)
//                         {
//                             obj.data[obj.i].response = obj.data[obj.i].response.substr(n).c_str();
//                             obj.data[obj.i].sendNext = false;
//                         }
//                         else
//                             obj.data[obj.i].sendNext = true;
//                     }
//                     else if (n == -1)
//                     {
//                         fds[obj.i].revents = POLLERR;
//                         obj.data[obj.i].sendNext = true;
//                         std::cout << "Send error\n";
//                     }
//                     n = 0;
//                 }
//                 else
//                 {
//                     close(fds[obj.i].fd); 
//                     fds.erase(fds.begin() + obj.i);
//                     std::cout << "The connection was cut off";
//                 }
//             }
//         }
//         else if (fds[obj.i].revents & POLLERR)
//         {
//             close(fds[obj.i].fd);
//             std::cout << "Socket error\n";
//         }
//         obj.i++;
//     }
//     close(server_fd);
// }




int main()
{
    Config obj;
    obj.config_file();
    // obj.get_server_name();

    // std::cout << "index = " << obj.get_index() << std::endl;
    std::cout << "autoindex = " << obj.autoIndex << std::endl;
    std::cout << "listen  = " << obj.get_listen_Ip()[0] << std::endl;
    std::cout << "listen  = " << obj.get_listen_Port()[0] << std::endl;

    // std::cout << "listen  = " << obj.get_listen_Ip()[1] << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Port()[1] << std::endl;

    // std::cout << "listen  = " << obj.get_listen_Ip()[2] << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Port()[2] << std::endl;

    std::cout << "ft_root  = " << obj.get_root() << std::endl;

    // std::cout << "ft_root  = " << obj.get_error_page(500) << std::endl;
    // std::cout << "ft_root  = " << obj.get_error_page(501) << std::endl;
    // std::cout << "index  = " << obj.get_index()[0] << std::endl;
    // std::cout << "index  = " << obj.get_index()[1] << std::endl;
    // std::cout << "index  = " << obj.get_index()[2] << std::endl;
    // std::cout << "index  = " << obj.get_index()[3] << std::endl;
    // std::cout << "ft_root  = " << obj.get_index() << std::endl;
    // std::cout << "server_name  = " << obj.get_server_name() << std::endl;
    // std::cout << "root  = " << obj.get_root() << std::endl;
    // std::cout << "methods  = " << obj.get_methods("GET") << std::endl;
    // std::cout << "methods  = " << obj.get_methods("POST") << std::endl;
    std::cout << "location_methods  = " << obj.get_methods("GET") << std::endl;
    std::cout << "location_methods  = " << obj.get_methods("POST") << std::endl;
    // std::cout << "error_page  = " << obj.get_error_page(404) << std::endl;
    // std::cout << "error_page  = " << obj.location[0].conf.get_error_page(404) << std::endl;
    // std::cout << "error_page  = " << obj.location[1].conf.get_error_page(404) << std::endl;
    // // std::cout << "error_page  = " << obj.location[2].conf.get_error_page(404) << std::endl;
    // std::cout << "location_methods  = " << obj.location[0].conf.get_methods_POST("POST") << std::endl;
    // std::cout << "location_methods  = " << obj.location[0].conf.get_methods("GET") << std::endl;
    // std::cout << "location_root = " << obj.location[0].conf.get_root() << std::endl;
    // std::cout << "location_root = " << obj.location[1].conf.get_root() << std::endl;
    // std::cout << "location_root = " << obj.location[2].conf.get_root() << std::endl;
    // std::cout << "index = " << obj.location[0].conf.get_index() << std::endl;
    // std::cout << "get_autoindex = " << obj.location[0].conf.get_autoindex() << std::endl;
}