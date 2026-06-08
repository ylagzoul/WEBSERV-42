#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <iostream>
#include <cstring>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <deque>
#include "../request/request.hpp"
#include "../response/Response.hpp"
#include "../config_file/config_file.hpp"
#include "state.h"
#include "Record.hpp"

#define DEFAULT_MAX_BODY_SIZE 1000000
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
    time_t time;
};
class Webserv;
class Server
{

    public:
        Server(std::vector<pollfd> &fds, int port);

        int server_fd;

        size_t iClnt;

        size_t clientId;
        // std::vector<pollfd> fds;
        std::vector<int> index_Client;
        sockaddr_in addr;
        socklen_t len;
        std::vector<Data> data;
        std::deque<Record> recs;
        std::vector<Request> req;
        // Mserver conf;

        void acceptNewCleint(Webserv &Web, pollfd &p, size_t &i);
        bool receiveRequest(Webserv &Web, pollfd &p, size_t &i, Config conf, SessionsList &sList);
        bool sendRespond(Webserv &Web, Server &obj, size_t &i, Config conf, pollfd &p);
        void close_client(Webserv &Web, pollfd &p, int i);
};

class Webserv
{
    public:
        Webserv();
        size_t s;
        std::vector<Server> server;
        std::vector<pollfd> fds;
        Mserver conf;
};

#endif