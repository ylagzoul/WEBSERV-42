#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>

class Webserv;
class Record;
class Server;
class Request;
class playloadData;


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
    char **cgiArgs;
    char **env;
    int canRead;

    pid_t pid;
    
    // -------------------------------------------------- //
    size_t bodyLen;
    bool dataReady;
    bool cgiValid;
    std::vector<std::string> headers;
    
    // --------------attributes for parsing-------------- //
    int status;
    int fd;
    std::string cgBuff;
    int cgStatusCode;
    std::string cgStatusName;
    std::map<std::string, std::string> cgHeaders;
    std::string         cgBodyFileName;
    std::string         bodyFile;
    std::string         fullLine;
    std::string         bodyPath;
    size_t              lenRead;
    int                 bodyFd;
    int                 getMove;
    bool                getEndHeaders;
    bool                finFlag;
    bool                getEndFile;
    size_t              contentLength;
    int                 countLenHead;
    double              chunkSize;
    t_client_state_cgi  clientState;
    t_req_state_cgi     reqState;
    int bodyType;
    bool             endRead;
    size_t           count;
};

// #include "../server/server.hpp"

int         cgiHandler(Webserv &Web, Record &log, Request req, std::vector<int> &listenPorts, size_t &i);
int         cgiProcess(Webserv &Web, Record &log, Request req, size_t &i);
int         parseCgiResponse(CgiHandler &cgiData, Request req);
void        returnedInternalError(CgiHandler &cgiData);
void        returnedBadError(CgiHandler &cgi);
std::string trim(const std::string& str);



#endif