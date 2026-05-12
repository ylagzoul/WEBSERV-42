#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "../server/server.hpp"

class CgiHandler
{
public:
    int fd[2];
    char **env;
    char *const *executeablePath;
    int cgStatusCode;
    std::string cgStatusName;
    std::string cgBuff;
    std::string path;
};

int cgiHandler(Server &obj, playloadData &data, Request &req, std::vector<int> &listenPort);

#endif
