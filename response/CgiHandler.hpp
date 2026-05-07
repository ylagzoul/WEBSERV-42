#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "Response.hpp"

class CgiHandler
{
    public:
        std::vector<std::string> headers;
};


int cgiHandler(playloadData &data, Request &req);

#endif