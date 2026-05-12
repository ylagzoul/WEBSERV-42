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
#include "Record.hpp"

#define DEFAULTBODY 100000000000
#define SIZESR 8192

// GET /file

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

    int server_fd;

    size_t iClnt;

    size_t clientId;
    std::vector<pollfd> fds;
    std::vector<int> index_Client;
    sockaddr_in addr;
    socklen_t len;
    std::vector<Data> data;
    std::vector<Record> recs;
    std::vector<Request> req;
    Config conf;

    void acceptNewCleint(pollfd &p, size_t &i);
    bool receiveRequest(pollfd &p, size_t &i);
    bool sendRespond(Server &obj, pollfd &p);
    void close_client(pollfd &p, int i);
};

#endif