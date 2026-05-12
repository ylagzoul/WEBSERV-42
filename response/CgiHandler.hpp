#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../server/server.hpp"


// typedef enum e_client_state
// {
//     CLIENT_OF,
//     CLIENT_ON,
//     CLIENT_ERROR

// } t_client_state;

// typedef enum e_req_state
// {
//     DEFAULT_ST = 10,
//     ONE_ST,
//     SECOND_ST,

// } t_req_state;

class CgiHandler
{
    public:
        // CgiHandler();
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

        // std::string bodyFile;
        // std::string fullLine;
        // std::string bodyPath;
        // size_t      lenRead;
        // int         bodyFd;
        // bool        getEndHeaders;
        // bool        finFlag;
        // bool        getEndFile;
        // size_t      contentLength;
        // t_client_state  clientState;
        // t_req_state     reqState;

};

int cgiHandler(CgiHandler &cgiData, Record &log, pollfd &pEvents, playloadData &data, Request &req);


#endif
