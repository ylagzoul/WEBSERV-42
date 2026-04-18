#ifndef RESPONSE_HPP
# define RESPONSE_HPP


#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>

//Tmp request//

class Config;
class Server;

class Request
{
    public:
        Request():bodyFile(0){}
        int status;
        std::string method;
        std::string path;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::ostream bodyFile;
        std::string fullLine;
        size_t size;
        bool getEndBody;
};

// Tmp configfile//

class payloadData
{
    public:
        std::string status_code;
        std::string message;
        std::string connection;
        std::string contentType;
        std::string realPath;
        std::string bodyStr;
        size_t bodySize;
        int listFd;
        std::map <std::string, std::string> mime;
};

void generateResponse(Request &req, payloadData &data, std::string &ret_resp);

std::string respond(Server serv, Config &conf);

#endif