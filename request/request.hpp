#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "Sessions.hpp"
#include "SessionsList.hpp"
#include <vector>
#include "../config_file/config_file.hpp"
#include "../response/Response.hpp"

using std::cout;

#define FIXED   1

typedef enum e_client_state
{
    CLIENT_OF,
    CLIENT_ON,
    CLIENT_ERROR

} t_client_state;

class Config;
class SessionsList;
// class Sessions;

class Request 
{
    public:
        Request();
        size_t bodyLen;
        std::string method;
        std::string path;
        std::string httpVersion;
        std::string host;
        std::map<std::string, std::string> headers;
        std::string bodyFile;
        std::string fullLine;
        std::string bodyPath;
        size_t      lenRead;
        int         countConLen;
        int         bodyFd;
        int         status;
        bool        getEndHeaders;
        bool        finFlag;
        bool        getEndFile;
        int         countLenHead;
        int         bodyType;
        size_t          contentLength;
        size_t          recLen;
        int getMove;
        size_t          maxBodySize;
        // size_t          countLenHead;
        t_client_state  clientState;
        Sessions    *session;
        std::map<std::string, std::string> cookies;
        void storeCookies(std::string value);
        void generateRandomName(std::string value);
        void clean();

};

void    readRequest(Request &req, char buffer[], size_t len, Config con, SessionsList &sList, playloadData &pdata);
std::string tolowerStr(std::string str);


#endif
