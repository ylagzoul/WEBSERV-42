#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <iostream>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

using std::cout;

class Request 
{
    public:
        Request();
        int status;
        std::string method;
        std::string path;
        std::string httpVersion;
        std::map<std::string, std::string>  headers;
        std::string bodyFile;
        std::string fullLine;

};
void    readRequest(Request &req, int fd);

#endif
