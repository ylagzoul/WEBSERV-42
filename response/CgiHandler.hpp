#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../server/server.hpp"

class CgiHandler
{
    public:
        int fd[2];
        const char **env;
        std::string path;
        std::string executeablePath;
};

int cgiHandler(pollfd &pEvents, playloadData &data, Request &req);


#endif
