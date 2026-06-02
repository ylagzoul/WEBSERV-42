#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>

typedef enum e_client_state_cgi
{
    CLIENT_CGI_OF = 20,
    CLIENT_CGI_ON,
    CLIENT_CGI_ERROR

} t_client_state_cgi;

typedef enum e_req_state_cgi
{
    DEFAULT_CGI_ST = 10,
    ONE_CGI_ST,
    SECOND_CGI_ST,

} t_req_state_cgi;

class CgiHandler
{
    public:
    CgiHandler();
    int bodyFd;
    char *const *executeablePath;
    std::string path;
    char *const *env;
    int canRead;
    
    // -------------------------------------------------- //
    bool dataReady;
    bool cgiValid;
    
    // --------------attributes for parsing-------------- //
    std::string cgBuff;
    int cgStatusCode;
    std::string cgStatusName;
    std::map<std::string, std::string> cgHeaders;
    std::string cgBodyFileName;

    std::string bodyFile;
    std::string fullLine;
    std::string bodyPath;
    size_t      lenRead;
    int fd;
    // int         bodyFd;
    bool        getEndHeaders;
    bool        finFlag;
    bool        getEndFile;
    size_t      contentLength;
    t_client_state_cgi  clientState;
    t_req_state_cgi     reqState;
    int bodyType;
};

#include "../server/server.hpp"


int cgiHandler(Record &log, CgiHandler &cgiData, Server &obj, playloadData &data, Request &req, std::vector<int> &listenPorts);

int cgiProcess(CgiHandler &cgiData, Record &log, Server &obj, playloadData &data, Request &req);

#endif