#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <iostream>
#include <map>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>

using std::cout;
#define FIXED   1
#define CHUNKED 2


class Request 
{
    public:
        Request();
        std::string method;
        std::string path;
        std::string httpVersion;
        std::string host;
        std::map<std::string, std::string>  headers;
        std::string bodyFile;
        std::string fullLine;
        int         status;
        bool        getEndHeaders;
        bool        finFlag;
        bool        getEndFile;
        int         bodyType;
        int         fdFile;
        int         contentLength;

};

void    readRequest(std::vector<Request> &req, int userId, char buffer[], size_t len);
std::string tolowerStr(std::string str);


#endif
