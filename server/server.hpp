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
#include "../request/request.hpp"
#include "../response/Response.hpp"
#include "../config_file/config_file.hpp"

typedef enum e_state
{
    NEW_RS = 100,
    ACTVE_RS = 101,
    CLOSED_RS = 102,
    GET_RS = 103,
    POST_RS = 104

} t_state;

class Record { 
    public:
        int fd;
        t_state status;
        t_state method;
};

class Data {
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

class Server {
    public:
        Server();
        size_t clientId;
        std::vector<Data> data;
};

#endif