#include "request.hpp"

bool isPrintable(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!isprint(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

bool isLfCr(const std::string &str)
{
     for (size_t i = 0; i < str.size(); i++)
    {
        if (isspace(static_cast<unsigned char>(str[i])))
            return false;
    }
    return (true);
}


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
        req.finFlag = true;
        return ;
    }
    req.method = array[0];
    req.path = array[1];
    req.httpVersion = array[2];
}

void    getRequestLine(Request &req)
{
    size_t pos = req.fullLine.find("\r\n");
    if (pos == std::string::npos)
        return ;
    fillCheck(req, req.fullLine.substr(0, pos));
    req.fullLine.erase(0, pos + 2);
}

void getHeaders(Request &req)
{
    size_t endLine = req.fullLine.find("\r\n");

    if (endLine == std::string::npos)
        return ;
    std::string line = req.fullLine.substr(0, endLine);
    if (line.empty())
    {
        req.getEndHeaders = true;
        req.fullLine.erase(0, endLine + 2);
        
        std::map<std::string,std::string>::iterator it;
        it = req.headers.find("transfer-encoding");
        if (req.headers.find("host") != req.headers.end())
            req.host = req.headers["host"];
        if (it != req.headers.end() && it->second == "chunked")
            req.bodyType = CHUNKED;
        else if (req.headers.find("content-length") != req.headers.end())
        {
            req.contentLength = std::atoi(req.headers["content-length"].c_str());
            req.bodyType = FIXED;
        }
        else
        {
            req.status = 400;
            req.finFlag = true;
            return ;
        }
        return ;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key   = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
        if (!isPrintable(key))
        {
            req.status = 400;
            req.finFlag = true;
            return ;
        }
        value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
        if (!isLfCr(value))
        {
            req.status = 400;
            req.finFlag = true;
            return ;
        }
        req.headers[tolowerStr(key)] = tolowerStr(value);
    }
    req.fullLine.erase(0, endLine + 2);
    return ;
}

// void getChunkedBody(Request &req)
// {
//     (void)fd;
//     std::vector<int> nb;
//     if (req.fdFile == -1)
//     {
//         req.fdFile = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
//         if (req.fdFile == -1)
//             throw std::runtime_error("Error file");
//     }
//     while (true)
//     {
//         if (std::strtol(req.fullLine.c_str(), NULL, 16) == 0 && req.fullLine.length() == 1)
//             req.fullLine.append("\n");
//         size_t pos = req.fullLine.find("\n");
//         if (pos == std::string::npos)
//         {
//             break ;
//         }
//         std::string sizeStr = req.fullLine.substr(0, pos);
//         size_t finalRead = req.fullLine.find("0\n\n");
//         // cout << finalRead << "=>" << req.fullLine << std::endl;
//         req.fullLine.erase(0, pos + 1);

//         int chunkSize = std::strtol(sizeStr.c_str(), NULL, 16);
//         nb.push_back(chunkSize);
//         if (chunkSize == 0)
//         {
//             if (finalRead == std::string::npos)
//             {
//                 req.status = 400;
//                 close(req.fdFile);
//                 throw std::runtime_error("Bad request");
//             }
//             req.fullLine.erase(0, 1);
//             req.getEndFile = true;
//             close(req.fdFile);
//             return ;
//         }
//         size_t pos2 = req.fullLine.find("\n");
//         std::string subStr = req.fullLine.substr(0, pos2);
//         if (subStr.length() < (size_t)chunkSize)
//         {
//             req.status = 400;
//             close(req.fdFile);
//             throw std::runtime_error("Bad request");
//         }
//         std::string data = req.fullLine.substr(0, chunkSize);
//         write(req.fdFile, data.c_str(), data.size());
//         req.fullLine.erase(0, pos2 + 1);
//         if (chunkSize != 0)
//             readRequest(req, fd);
//     }
//     // I Have new logic is made Here ;
//     int i = 0;
//     while (i < nb.size())
//     {
//         if (nb[i] == 0)
//             break;
//         i++;
//     }
//     if ( i != nb.size() )
//         readRequest(req, fd);
// }

void    getFixedBody(Request &req)
{
    static int bodyLen;
    if (req.fdFile == -1)
    {
        req.fdFile = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (req.fdFile == -1)
        {
            std::cerr << "Error opening file\n";
            throw std::runtime_error("Error file \n");
            return ;
        }
    }
    bodyLen += req.fullLine.length();
    if (bodyLen < req.contentLength)
    {
        if (bodyLen != 0)
        {
            write(req.fdFile, req.fullLine.c_str(), req.fullLine.length());
            req.fullLine.clear();
        }
        return ;
    }
    int sizePa = (req.contentLength+req.fullLine.length())-bodyLen;
    std::string d = req.fullLine.substr(0, sizePa);
    
    write(req.fdFile, d.c_str(), sizePa);
    close(req.fdFile);
    req.getEndFile = true;
}

void    readRequest(std::vector<Request> &req, int userId, char buffer[], size_t len)
{

    req[userId].fullLine.append(buffer, len);
    while (1)
    {
        if (req[userId].method.empty())
            getRequestLine(req[userId]);
    
        if (req[userId].getEndHeaders == false)
            getHeaders(req[userId]);

        if (req[userId].method == "POST" && req[userId].getEndFile == false && req[userId].bodyType == FIXED)
            getFixedBody(req[userId]);
        // else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        //     getChunkedBody(req);
        if (req[userId].getEndFile || req[userId].method != "POST")
            req[userId].finFlag = true;
        if (req[userId].finFlag)
            break ;
    }
    return ;
}



int main()
{
    std::vector<Request> req;
    // char buffer[8000];

    int fd = open("httpRequest.http", O_RDONLY, 0644);
    if (fd == -1)
    {
        cout << "fd Error" << std::endl;
        return 1;
    }
    try
    {
        for (int i = 0; i<5; i++)
            req.push_back(Request());
        // recv(soket_fd, buffer, 8000, 0);
        int userId = 0;
        char buffer2[] = "POST / HTTP/1.0\r\n\
Host: localhost:5500\r\n\
Connection: keep-alive\r\n\
Cache-Control: max-age=0\r\n\
sec-ch-ua: Brave;v=143, Chromium;v=143, Not A(Brand;v=24\r\n\
Content-Length: 10\r\n\
sec-ch-ua-mobile: ?0\r\n\
sec-ch-ua-platform: Linux\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36\r\n\
Origin: http://localhost:6600\r\n\
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryzQKhhmwY0ASUm66m\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n\
Sec-GPC: 1\r\n\
Accept-Language: en-US,en;q=0.6\r\n\
Sec-Fetch-Site: same-site\r\n\
Sec-Fetch-Mode: navigate\r\n\
Sec-Fetch-User: ?16assfa\r\n\
Sec-Fetch-Dest: document\r\n\
Referer: http://localhost:6600/\r\n\r\n\
hamza mohammed youssef boha abde allah";

        std::string buf = buffer2;
        
        readRequest(req, userId, buffer2, buf.length());
        if (req[userId].finFlag == true)
            cout << "We finish correctly" << std::endl;
        else
            cout << "We still wait" << std::endl;
            
        cout << "method: " << req[userId].method << std::endl;
        cout << "path: " << req[userId].path << std::endl;
        cout << "httpVersion: " << req[userId].httpVersion << std::endl;
        for (std::map<std::string, std::string>::iterator it = req[userId].headers.begin();
         it != req[userId].headers.end();
            ++it)
        {
            std::cout << it->first << ":" << it->second << std::endl;
        }
        // cout << req.fullLine;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}
