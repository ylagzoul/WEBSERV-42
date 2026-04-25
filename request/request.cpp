#include "../server/server.hpp"

Request::Request() : status(200), getEndHeaders(false), getEndFile(false), fdFile(-1), clientState(CLIENT_OF), reqState(DEFAULT_ST){};

std::string tolowerStr(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}

bool checkPath(std::string &path)
{
    std::stringstream iss(path);
    std::string        splitedPath;
    std::string         tmp;

    if (path.at(0) != '/')
        return (false);
    while (getline(iss, splitedPath, '/'))
    {
        if (!splitedPath.empty())
        {
            tmp.append("/");
            tmp += splitedPath;
        }
    }
    path = tmp;
    return (true);
}

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
    if (!checkPath(req.path))
    {
        req.status = 400;
        req.finFlag = true;
        return ;
    } 
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
        req.fullLine.erase(0, endLine + 4);
        
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





void    getFixedBody(Request &req, Config con)
{
    static size_t bodyLen;
    if (req.fdFile == -1)
    {
        req.fdFile = open("out.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (req.fdFile == -1)
        {
            std::cerr << "Error opening file\n";
            throw std::runtime_error("Error file \n");
            return ;
        }
    }
    bodyLen += req.fullLine.length();
    if (bodyLen > con.client_max_body_size)
    {
        req.status = 400;
        req.getEndFile = true;
        return ;
    }
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

void    readRequest(std::vector<Request> &req, int userId, char buffer[], size_t len, Config con)
{
    req[userId].fullLine.append(buffer, len);
    while (1)
    {
        if (req[userId].method.empty())
            getRequestLine(req[userId]);
        size_t pos = req[userId].fullLine.find("\r\n\r\n");
        if (pos == std::string::npos)
            break ;
        if (req[userId].getEndHeaders == false)
            getHeaders(req[userId]);
        if (req[userId].method == "POST" && req[userId].getEndFile == false && req[userId].bodyType == FIXED)
            getFixedBody(req[userId], con);
        // else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        //     getChunkedBody(req);
        if (req[userId].getEndFile || req[userId].method != "POST")
            req[userId].finFlag = true;
        if (req[userId].finFlag)
        {
            req[userId].clientState = CLIENT_ON;
            break ;
        }
    }
    return ;
}
