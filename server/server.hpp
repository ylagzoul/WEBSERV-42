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

#define DEFAULT_MAX_BODY_SIZE 100000000
#define SIZESR 8192

// GET /file
void closeCGI(Webserv &Web, CgiHandler &cgiData, std::string method, size_t &i);

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
        Server(Webserv &Web, int port, const std::string &IP);
        std::vector<Server> serverPort;

        int server_fd;
        std::vector<int> index_Client;
        sockaddr_in addr;
        socklen_t len;
        size_t ii;


        void acceptNewCleint(Webserv &Web);
        bool receiveRequest(Webserv &Web, size_t &i, SessionsList &sList);
        bool sendRespond(Webserv &Web, size_t &i);
        void close_client(Webserv &Web, size_t &indexClnt);
};

class Webserv
{
public:
    Webserv(std::string nameFielConfig);
    size_t s;
    size_t n;
    int indexC;
    std::vector<Server> server;
    std::vector<Data> data;
    std::deque<Record> recs;
    std::vector<Request> req;
    std::vector<pollfd> fds;
    std::map<int, std::string> typeFd;
    std::map<int, std::string> statusFd;
    std::vector<int> reuseFd;
    
    int count;
    std::vector<Config> config;
    void config_file(std::string nameFielConfig);

};

#endif