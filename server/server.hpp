#ifndef SERVER_HPP
#define SERVER_HPP

// reqoust

#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <map>
#include <cstdlib>
#include <arpa/inet.h>
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

void closeCGI(Webserv &Web, CgiHandler &cgiData, std::string method, size_t &i);


class Webserv;

class Server
{

public:
    Server(Webserv &Web, int port, const std::string &IP);
    int server_fd;

    std::vector<Server> serverPort;
    std::vector<int> index_Client;
    std::vector<int> reuseFd;
    sockaddr_in addr;
    socklen_t len;

    void acceptNewClient(Webserv &Web);
    bool receiveRequest(Webserv &Web, size_t &indexClnt, SessionsList &sList);
    bool sendRespond(Webserv &Web, size_t &indexClnt, SessionsList &sList);
    void close_client(Webserv &Web, size_t &indexClnt);
};

class Webserv
{
public:
    Webserv(std::string configNameFile);

    int                     count;
    int                   indexC;
    size_t                s;
    size_t                t;
    size_t                i;
    size_t                n;
    std::vector<Server>         server;
    std::deque<Record>          recs;
    std::vector<Request>        req;
    std::vector<pollfd>         fds;
    std::map<int, std::string>  typeFd;
    std::map<int, std::string>  statusFd;

    std::vector<Config> config;
    void config_file(std::string configNameFile);
};

#endif