#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
// #include "../request/request.hpp"
#include "../response/Response.hpp"
#include "../config_file/config_file.hpp"

class Data
{
public:
    Data();
    std::string response;
    int maxSend;
    bool sendNext;
    bool rspEnd;
    std::string reqEnd;
    int recvSize;
    int recvNum;
    bool recvNext;
};

class Server
{
public:
    Server();
    size_t i;
    std::vector<Data> data;
};

#endif