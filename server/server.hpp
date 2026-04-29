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
#include "state.h"

# define DEFAULTBODY 1024

class Record { 
    public:
        Record(){number = 0; fileFd = -1; fd = -1; status = NEW_RS;/* method = NONE_RS;*/}
        int fd;
        int fileFd;
        int number;
        DIR *dir;
        t_state status;
        t_state method;
        playloadData data;
        std::string buffer;
        std::string fileName;
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