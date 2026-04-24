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
    ACTIVE_RS,
    CLOSED_RS,
    GET_RS,
    POST_RS,
    ERROR_RS

} t_state;

class Record { 
    public:
        Record(){number = 0; fd = -1; status = NEW_RS;};
        int number;
        int fd;
        t_state status;
        t_state method;
        std::string buffer;
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
        Server(){}//attention /!\;
        size_t clientId;
        std::vector<Data> data;
};

#endif