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


using namespace std;

int main()
{
    vector<Request> req;
    Config con;
    con.client_max_body_size = 14;
    for (size_t i = 0; i < 3; i++)
        req.push_back(Request());
    int i = 0;
        char buffer[] = "POST /paj//home HTTP/1.0\r\n\
Content-Length: 2\r\n\
\r\n\r\nkd";

    string str = buffer;
    while (i < 1)
    {
        readRequest(req, i, buffer, str.length(), con);
        i++;
    }


    cout << req[0].status << endl;
    cout << req[0].method << endl;
    cout << req[0].clientState << endl;


}

// int main()
// {
//     Config obj;
//     obj.config_file();
    // obj.get_server_name();

    // std::cout << "index = " << obj.get_index() << std::endl;
    // std::cout << "autoindex = " << obj.autoIndex << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Ip()[0] << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Port()[0] << std::endl;

    // std::cout << "listen  = " << obj.get_listen_Ip()[1] << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Port()[1] << std::endl;

    // std::cout << "listen  = " << obj.get_listen_Ip()[2] << std::endl;
    // std::cout << "listen  = " << obj.get_listen_Port()[2] << std::endl;

    // std::cout << "ft_root  = " << obj.get_root() << std::endl;

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
    // std::cout << "location_methods  = " << obj.get_methods("GET") << std::endl;
    // std::cout << "location_methods  = " << obj.get_methods("POST") << std::endl;
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
// }



// int main()
// {
//     Config obj;
//     obj.config_file();
//     obj.get_server_name();

//     // std::cout << "index = " << obj.get_index() << std::endl;
//     // std::cout << "autoindex = " << obj.autoIndex << std::endl;
//     // std::cout << "listen  = " << obj.get_listen_Ip()[0] << std::endl;
//     // std::cout << "listen  = " << obj.get_listen_Port()[0] << std::endl;

//     // std::cout << "listen  = " << obj.get_listen_Ip()[1] << std::endl;
//     // std::cout << "listen  = " << obj.get_listen_Port()[1] << std::endl;

//     // std::cout << "listen  = " << obj.get_listen_Ip()[2] << std::endl;
//     // std::cout << "listen  = " << obj.get_listen_Port()[2] << std::endl;

//     // std::cout << "ft_root  = " << obj.get_root() << std::endl;

//     std::cout << "ft_root  = " << obj.get_error_page(500) << std::endl;
//     std::cout << "ft_root  = " << obj.get_error_page(501) << std::endl;
//     // std::cout << "index  = " << obj.get_index()[0] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[1] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[2] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[3] << std::endl;
//     // std::cout << "ft_root  = " << obj.get_index() << std::endl;
//     // std::cout << "server_name  = " << obj.get_server_name() << std::endl;
//     // std::cout << "root  = " << obj.location[0].conf.get_root() << std::endl;
//     // std::cout << "methods  = " << obj.get_methods("GET") << std::endl;
//     // std::cout << "methods  = " << obj.get_methods("POST") << std::endl;
//     // std::cout << "methods  = " << obj.get_methods("DELETE") << std::endl;
//     // std::cout << "root  = " << obj.get_root() << std::endl;
//     std::cout << "-----------------------------------\n";
//     // std::cout << "location_root = " << obj.location[0].conf.get_methods("GET") << std::endl;
//     // std::cout << "location_root = " << obj.location[0].conf.get_methods("POST") << std::endl;

//         for(size_t i = 0; i < obj.location.size();i++)
//         {
//             // if (obj.location[i].location_name == "/upload")
//             // {
//                 std::cout << "ft_root  = " << obj.location[i].conf.get_error_page(510) << std::endl;
//                 std::cout << "ft_root  = " << obj.location[i].conf.get_error_page(506) << std::endl;
//                 std::cout << "-----------------------------------\n";
//                 std::cout << "-----------------------------------\n";
//             // }
//         }
//     // std::cout << "location_methods  = " << obj.get_methods("GET") << std::endl;
//     // std::cout << "location_methods  = " << obj.get_methods("POST") << std::endl;
//     // std::cout << "error_page  = " << obj.get_error_page(404) << std::endl;
//     // std::cout << "error_page  = " << obj.location[0].conf.get_error_page(404) << std::endl;
//     // std::cout << "error_page  = " << obj.location[1].conf.get_error_page(404) << std::endl;
//     // std::cout << "error_page  = " << obj.location[2].conf.get_error_page(404) << std::endl;
//     // std::cout << "location_methods  = " << obj.location[0].conf.get_methods_POST("POST") << std::endl;
//     // std::cout << "location_methods  = " << obj.location[0].conf.get_methods("GET") << std::endl;
//     // std::cout << "location_root = " << obj.location[0].conf.get_root() << std::endl;
//     // std::cout << "location_root = " << obj.location[1].conf.get_root() << std::endl;
//     // std::cout << "location_root = " << obj.location[2].conf.get_root() << std::endl;
//     // std::cout << "index = " << obj.location[0].conf.get_index() << std::endl;
//     // std::cout << "get_autoindex = " << obj.get_autoindex() << std::endl;
//     // std::cout << "get_autoindex = " << obj.location[0].conf.get_autoindex() << std::endl;
// }