#include "request.hpp"

void    getRequestLine(Request &req, int fd)
{
    (void) req;
    if (req.fullLine.find('\n') == std::string::npos)
    {
        readRequest(req, fd);
        std::stringstream splitReq(req.fullLine);
        std::string slice;
        std::vector<std::string> v1;
        while(splitReq >> slice)
            v1.push_back(slice);
        if (v1[0] != "POST" && v1[0] != "GET" && v1[0] != "DELETE")
        {
            req.status = 400;
            throw std::runtime_error("Bad Request");
        }
        req.method = v1[0];
        return ;
    }
}

void    readRequest(Request &req, int fd)
{
    char buffer[8000];
    int res = read(fd, buffer, 5);
    if (res < 0)
    {
        cout << "read error" << std::endl;
        return ;
    }
    req.fullLine += buffer;
    getRequestLine(req, fd);
}



int main()
{
    Request req;
    int fd = open("httpRequest.http", O_RDONLY, 0644);
    if (fd == -1)
    {
        cout << "fd Error" << std::endl;
        return 1;
    }
    try
    {
        readRequest(req, fd);
        cout << req.fullLine << std::endl;
        cout << req.method << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}