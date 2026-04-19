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

class Config;
class Server;
class Record;
class Request;

class playloadData
{
    public:
        std::string status_code;
        std::string message;
        std::string connection;
        std::string contentType;
        std::string realPath;
        std::string bodyStr;
        std::string location;
        std::map<std::string, bool> locationMethods;
        size_t bodySize;
        int listFd;
        std::map <std::string, std::string> mime;
};

void generateResponse(Request &req, playloadData &data, std::string &ret_resp);

std::string respond(Record &rec, Server serv, Config &conf, Request &req);

#endif