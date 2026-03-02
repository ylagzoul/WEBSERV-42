#include "request.hpp"

void    fillCheck(Request &req, std::string reqLine)
{
    std::stringstream splitReq(reqLine);
    std::string slice;
    std::vector<std::string> array;
    while(splitReq >> slice)
        array.push_back(slice);
    if (array.size() != 3 || (array[0] != "POST" && array[0] != "GET" && array[0] != "DELETE"))
    {
        req.status = 400;
        throw std::runtime_error("Bad Request");
    }
    if (array[2] != "HTTP/1.1" && array[2] != "HTTP/1.0")
    {
        req.status = 400;
        throw std::runtime_error("Bad Request");
    }
    req.method = array[0];
    req.path = array[1];
    req.httpVersion = array[2];
}

void    getRequestLine(Request &req, int fd)
{
    size_t pos = req.fullLine.find('\n');
    if (pos == std::string::npos)
    {
        readRequest(req, fd);
        return ;
    }
    fillCheck(req, req.fullLine.substr(0, pos));
    req.fullLine.erase(0, pos + 1);
}

void    getHeaders(Request &req, int fd)
{
    size_t pos = req.fullLine.find("\n\n");
    size_t pos2 = req.fullLine.find('\n');

    if (pos == std::string::npos)
    {
        if (pos2 != std::string::npos)
        {
            std::string tmp = req.fullLine;
            if (tmp.length() == 1 || tmp[0] == '\n')
            {
                req.getEndBody = true;
                req.fullLine.erase(0, pos + 2);
                return ;
            }
            cout << tmp << " = " << tmp.length() << std::endl;
            char        del = ':';

            if (tmp.substr(0, pos2).find(del) != std::string::npos)
            {
                std::string key = tmp.substr(0, tmp.find(del));
                std::string value = tmp.substr(tmp.find(del) + 1, pos2);
                req.headers[tmp.substr(0, tmp.find(del))] = tmp.substr(tmp.find(del) + 1, pos2);
                // req.headers.insert(std::make_pair(tmp.substr(0, tmp.find(del)), tmp.substr(tmp.find(del) + 1, pos2)));
                req.fullLine.erase(0, pos2 + 1);
            }  
        }
        readRequest(req, fd);
        return ;
    }
}

void    getBody(Request &req, int fd)
{
    static int bodyLen;
    int contentLength;
    std::istringstream iss(req.headers["Content-Length"]);
    iss >> contentLength;
    if (req.fdFile == -1)
        req.fdFile = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (req.fdFile == -1)
    {
        std::cerr << "Error opening file\n";
        throw std::runtime_error("Error file \n");
        return ;
    }
    bodyLen += req.fullLine.length();
    if (bodyLen < contentLength)
    {
        if (bodyLen != 0)
            write(req.fdFile, req.fullLine.c_str(), req.fullLine.length());
        cout << "fullLine 1 :" << req.fullLine.c_str() << std::endl;
        req.fullLine.clear();
        readRequest(req, fd);
        return ;
    }
    int sizePa = (contentLength+req.fullLine.length())-bodyLen;
    std::string d = req.fullLine.substr(0, sizePa);
    
    cout << "fullLine 2 :" << d << std::endl;
    write(req.fdFile, d.c_str(), sizePa);
    close(req.fdFile);
    req.getEndFile = true;
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
    req.fullLine.append(buffer, res);
    if (req.method.empty())
        getRequestLine(req, fd);
    if (req.getEndBody == false)
        getHeaders(req, fd);
    // if (req.method == "POST" && req.getEndFile == false)
    //     getBody(req, fd);
    
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
        // cout << req.fullLine;
        // cout << "method: " << req.method << std::endl;
        // cout << "path: " << req.path << std::endl;
        // cout << "httpVersion: " << req.httpVersion << std::endl;
        // for (std::map<std::string, std::string>::iterator it = req.headers.begin();
        //  it != req.headers.end();
        //     ++it)
        // {
        //     std::cout << it->first << ":" << it->second;
        // }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}