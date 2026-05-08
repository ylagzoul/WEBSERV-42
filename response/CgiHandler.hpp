#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "Response.hpp"

class CgiHandler
{
    public:
        const char **env;
        int fd[2];
        std::string path;
        std::string executeablePath;
};

int cgiHandler(playloadData &data, Request &req);


#endif