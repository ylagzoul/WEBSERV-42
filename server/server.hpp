#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
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

#define MAX_LEN_HEADERS_FEILD 16 * 1024
#define MAX_LEN_PATH 8 * 1024
#define MAX_LEN_HEADER_FEILD 2 * 1024

#define DEFAULT_MAX_BODY_SIZE 100000000000000
#define SIZESR 8192

// GET /file
void closeCGI(Webserv &Web, Server &obj, std::string method);

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
        Server(std::vector<pollfd> &fds, int port, const std::string &IP);
        std::vector<Server> serverPort;

        int server_fd;

        size_t iClnt;
        pid_t pid;

        size_t clientId;
        std::vector<int> index_Client;
        sockaddr_in addr;
        socklen_t len;
        std::vector<Data> data;
        std::deque<Record> recs;
        std::vector<Request> req;

        void acceptNewCleint(Webserv &Web, pollfd &p, size_t &i);
        bool receiveRequest(Webserv &Web, pollfd &p, size_t &i, SessionsList &sList);
        bool sendRespond(Webserv &Web, Server &obj, size_t &i, pollfd &p);
        void close_client(Webserv &Web, pollfd &p, size_t &i);
};

class Webserv
{
public:
    Webserv();
    size_t s;
    int indexC;
    std::vector<Server> server;
    std::vector<pollfd> fds;

    // config file
    int count;
    std::vector<Config> config;
    void config_file();

};

#endif