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
#include "../server/state.h"

class Config;
class Record;
class Request;
class Server;

class playloadData
{
    public:
        std::string status_code;
        std::string resMessage;
        std::string connection;
        std::string contentType;
        std::string realPath;
        std::string bodyStr;
        std::string error;
        std::string uri;
        std::vector <std::string> index;
        std::map<int, std::string> errorPages;
        std::string location;
        std::map<std::string, bool> locationMethods;
        std::string locationRoot;
        std::string locationUpload;
        size_t bodySize;
        int autoIndex;
        int locationIndex;
        int listFd;
        int uploadOn;
        std::map <std::string, std::string> mime;
};


void generateResponse(playloadData &data, std::string &ret_resp, Record &log, t_state pastMethod);

std::string respond(playloadData &data, Record &rec, Config &conf, Request &req); // mrege

#endif