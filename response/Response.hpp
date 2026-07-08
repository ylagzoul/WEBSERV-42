#ifndef RESPONSE_HPP
#define RESPONSE_HPP


#include "../server/state.h"
#include "../request/request.hpp"

class Config;
class Record;   
class Request;
class Server;
class Webserv;
class SessionsList;

class playloadData
{
public:
    int statusCode;
    std::string statusName;
    std::string connection;
    std::string contentType;
    std::string realPath;
    std::string errorBody;
    std::string error;
    std::string uri;
    std::vector<std::string> index;
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
    std::string retPath;
    std::string loginToken;
    std::vector<std::string> redirectTokens;
    int retCode;
    size_t bodySize;

    bool autoIndex;
    bool isCgi;
    bool retValid;
    bool uploadOn;
    int listFd;
    int cgiDataPos;



    std::map<std::string, std::string> mime;
    std::map<int, std::string> table;
};

#include "CgiHandler.hpp"

bool getFile(Record &log);
std::string getRandName();
std::string cleanPath(std::string fullPath);
void findLongestMatch(playloadData &data, Config &conf, std::string reqPath);
void responseGenerator(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Record &log, Request &req);
std::string respond(Webserv &Web, Record &log, Config &conf, Request &req, size_t &i);




#endif