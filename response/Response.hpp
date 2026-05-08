#ifndef RESPONSE_HPP
# define RESPONSE_HPP


#include <iostream>
#include <stdlib.h>
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
#include "../request/request.hpp"

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
        std::vector<std::string> cgiExt;
        std::vector<std::string> cgiPath;
        std::map<int, std::string> errorPages;
        std::string location;
        std::map<std::string, bool> methods;
        std::string root;
        std::string upload;
        std::string cgiInfo;
        std::string cgiScriptFile;
        std::string cgiFile;
        std::string cgiExecutePath;
        size_t bodySize;
        
        int autoIndex;
        bool isCgi;
        int listFd;
        int uploadOn;
        int cgiDataPos;
        std::map <std::string, std::string> mime;
};

#include "CgiHandler.hpp"

void generateResponse(playloadData &data, std::string &ret_resp, Record &log, t_state pastMethod);

std::string respond(playloadData &data, Record &log, Config &conf, Request &req);

#endif