#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../server/server.hpp"


// typedef enum e_client_state_cgi
// {
//     CLIENT_CGI_OF,
//     CLIENT_CGI_ON,
//     CLIENT_CGI_ERROR

// } t_client_state_cgi;

// typedef enum e_req_state_cgi
// {
//     DEFAULT_CGI_ST = 10,
//     ONE_CGI_ST,
//     SECOND_CGI_ST,

// } t_req_state_cgi;

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
        // t_client_state_cgi  clientState;
        // t_req_state_cgi     reqState;
        // int bodyType;

};

int cgiHandler(CgiHandler &cgiData, Record &log, pollfd &pEvents, playloadData &data, Request &req);


#endif
