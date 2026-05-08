#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <iostream>
#include <map>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include "../config_file/config_file.hpp"

using std::cout;
#define FIXED   1
#define CHUNKED 2

typedef enum e_client_state
{
    CLIENT_OF,
    CLIENT_ON,
    CLIENT_ERROR

} t_client_state;

typedef enum e_req_state
{
    DEFAULT_ST = 10,
    ONE_ST,
    SECOND_ST,

} t_req_state;

class Config;

class Request 
{
    public:
        Request();
        std::string method;
        std::string path;
        std::string httpVersion;
        std::string host;
        std::map<std::string, std::string> headers;
        std::string bodyFile;
        std::string fullLine;
        std::string bodyPath;
        int         bodyFd;
        int         status;
        bool        getEndHeaders;
        bool        finFlag;
        bool        getEndFile;
        int         bodyType;
        size_t          contentLength;
        t_client_state  clientState;
        t_req_state     reqState;


        void clean();

};

void    readRequest(std::vector<Request> &req, int userId, char buffer[], size_t len, Config con);
std::string tolowerStr(std::string str);


#endif
