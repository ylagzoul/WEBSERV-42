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

// GET /file


// Record::~Record()
// {
//     if (fileFd != -1)
//         close(fileFd);
//     if (fd != -1)
//         close(fd);
//     if (dir)
//         closedir(dir);
// }

class Record { 
    public:
        Record()
        {
            number = 0;
            fileFd = -1;
            fd = -1;
            dir = NULL;
            status = NEW_RS;
            method = NO_MD;
        }
        int fd;
        int fileFd;
        int number;
        DIR *dir;
        t_state status;
        t_state method;
        playloadData data;
        std::string fileName;
        std::string buffer;
        ~Record(){}
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

        int server_fd;

        size_t                  clientId;
        std::vector<pollfd>     fds;
        sockaddr_in             addr;
        socklen_t               len;
        std::vector<Data>       data;
        std::vector<Record>     recs;
        std::vector<Request>    req;
        Config                  conf;

        void acceptNewCleint(pollfd &p, size_t &i);
        bool receiveRequest(pollfd &p, size_t &i);
        bool sendRespond(pollfd &p, size_t &i);
        void close_client(pollfd &p, int i);
};

#endif