#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../server/server.hpp"

class CgiHandler
{
    public:
    const char **env;
    std::string path;
    std::string executeablePath;
    int canRead;
    // --------------attributes for parsing-------------- //
    std::string cgBuff;//READING
    std::map<std::string, std::string> cgHeaders;
    int cgStatusCode;
    std::string cgStatusName;
    std::string cgBodyFileName;
};

int cgiHandler(CgiHandler &cgiData, Record &log, pollfd &pEvents, playloadData &data, Request &req);


#endif
