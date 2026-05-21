#include "../server/server.hpp"

Request::Request() : lenRead(0), bodyFd(-1), status(200), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_OF), reqState(DEFAULT_ST) {};

bool checkDigits(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int nb = str.at(i);
        if (!isdigit(nb))
            return (false);
    }
    return (true);
}

bool isHoldSpace(const std::string& str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (std::isspace(str.at(i)))
            return (false);
    }
    return (true);
}

std::string trim(const std::string& str)
{
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start]))
        start++;

    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1]))
        end--;

    return str.substr(start, end - start);
}

void    Request::storeCookies(std::string value)
{
    std::stringstream iss(value);
    std::string tmp;
    while (getline(iss, tmp, ';'))
    {
        size_t equal = tmp.find('=');
        if (equal != std::string::npos)
        {
            std::string key = tmp.substr(0, equal);
            std::string value = tmp.substr(equal + 1);
            if (key == "session_id")
            {
                this->cookies[key] = value;
                return ;
            }
        }
    }
}
std::string tolowerStr(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
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

std::string toStr(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
}

int    count_space(std::string str)
{
    int count = 0;
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == ' ')
            count++;
    }
    return (count);
}

bool isHex(char h)
{
    return ( h >= '0' && h <= '9' ) ||
           ( h >= 'a' && h <= 'f' ) ||
           ( h >= 'A' && h <= 'F' ); 
}

int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    return (-1);
}

char hexToChar(char a, char b)
{
    int high = hexValue(a);
    int low  = hexValue(b);

    return ((high << 4) | low);
}

bool checkPrecentEnc(std::string &pathSlice)
{
    std::string copyPath = pathSlice;
    pathSlice.clear();
    for (size_t i = 0; i < copyPath.length(); i++)
    {
        if (copyPath.at(i) == '%')
        {
            // std::cout << "Found % at index: " << i << std::endl;
            if(!copyPath[i + 1] || !copyPath[i + 2])
                return (false);
            if (!isHex((char)copyPath.at(i+1)) || !isHex((char)copyPath.at(i+2)))
                return(false);
            char res = hexToChar(copyPath.at(i+1), copyPath.at(i+2));
            if ((unsigned char)res > 127)
                return false;
            pathSlice += res;
            i += 2;
        }
        else
        {
            pathSlice += copyPath.at(i);
        }
    }
    return (true);
}

bool checkPath(std::string &path)
{
    std::stringstream iss(path);
    std::string splitedPath;
    std::string tmp;

    if (path.at(0) != '/' && path.at(0) != '%')
        return (false);
    else if (path.at(0) == '/' && path.length() == 1)
        return (true);
    while (getline(iss, splitedPath, '/'))
    {
        if (!splitedPath.empty())
        {
            // cout << splitedPath << std::endl;
            if(!isPrintable(splitedPath))
                return (false);
            if (!checkPrecentEnc(splitedPath))
                return (false);
        }
        tmp += "/";
        tmp += splitedPath;
    }
    path = tmp;
    path.erase(0, 1);
    if (path.length() == 1 && path != "/")
        return (false);
    else
        return (true);
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

void fillCheck(Request &req, std::string reqLine)
{
    std::stringstream splitReq(reqLine);
    std::string slice;
    std::vector<std::string> array;
    if (count_space(reqLine) > 2)
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    while (splitReq >> slice)
        array.push_back(slice);
    if (array.size() != 3)
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    if (array[2] != "HTTP/1.1" && array[2] != "HTTP/1.0")
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    req.method = array[0];
    req.path = array[1];
    req.httpVersion = array[2];
    if (!checkPath(req.path))
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
}

void getRequestLine(Request &req)
{
    size_t pos = req.fullLine.find("\r\n");
    if (pos == std::string::npos)
        return;
    fillCheck(req, req.fullLine.substr(0, pos));
    req.lenRead += req.fullLine.substr(0, pos + 2).length();
    req.fullLine.erase(0, pos + 2);
    // cout << "size : " << req.fullLine.size() << " content = "<< req.fullLine << std::endl;
    cout << req.path << std::endl;
}

void getHeaders(Request &req)
{
    size_t endLine = req.fullLine.find("\r\n");
    if (endLine == std::string::npos)
        return;
    std::string line = req.fullLine.substr(0, endLine);
    if (line.empty())
    {
        req.getEndHeaders = true;
        req.lenRead += req.fullLine.substr(0, endLine + 2).length();
        req.fullLine.erase(0, endLine + 2);
        std::map<std::string, std::string>::iterator it;
        it = req.headers.find("transfer-encoding");
        if (req.headers.find("host") != req.headers.end())
            req.host = req.headers["host"];
        if (req.headers.find("set-cookie") != req.headers.end())
            req.storeCookies(req.headers["set-cookie"]);
        if (it != req.headers.end() && it->second == "chunked")
            req.bodyType = CHUNKED;
        else if (req.headers.find("content-length") != req.headers.end() && checkDigits(req.headers["content-length"]))
        {
            req.contentLength = std::atoi(req.headers["content-length"].c_str());
            req.bodyType = FIXED;
        }
        else if (req.method != "GET")
        {
            req.status = 400;
            req.finFlag = true;
            return ;
        }
        return;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (!isPrintable(key) || !isHoldSpace(key))
        {
            req.status = 400;
            req.finFlag = true;
            return;
        }
        value = trim(value);
        req.headers[tolowerStr(key)] = value;
    }
    else
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    req.lenRead += req.fullLine.substr(0, endLine + 2).length();
    req.fullLine.erase(0, endLine + 2);
    return;
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

void getFixedBody(Request &req, Config con)
{
    (void)con;
    static size_t bodyLen;
    req.lenRead += req.fullLine.length();
    if (req.bodyFd == -1)
    {
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (req.bodyFd == -1)
        {
            std::cerr << "Error opening file\n";
            req.status = 500;
            return ;
        }
    }
    bodyLen += req.fullLine.length();
    if (bodyLen > 10000000) // after handel
    {
        req.status = 400;
        req.getEndFile = true;
        return;
    }
    if (bodyLen < req.contentLength)
    {
        if (bodyLen != 0)
        {
            write(req.bodyFd, req.fullLine.c_str(), req.fullLine.length());
            req.fullLine.clear();
        }
        return;
    }
    int sizePa = (req.contentLength + req.fullLine.length()) - bodyLen;
    std::string d = req.fullLine.substr(0, sizePa);

    write(req.bodyFd, d.c_str(), sizePa);
    close(req.bodyFd);
    req.getEndFile = true;
}

void    generateCookies(Request &req, SessionsList &sList)
{
    if (req.session)
        cout << "last Geneating: " << req.session->userName << " -- " << req.session->lastLog << std::endl;
    if (!req.headers["set-cookie"].empty())
        cout << "Last Header: " <<  req.headers["set-cookie"] << std::endl;
    sList.generateSession(req);
    if (req.session)
        cout << "New Geneating: " << req.session->userName << " -- " << req.session->lastLog << std::endl;
    if (!req.headers["set-cookie"].empty())
        cout << "New Header: " <<  req.headers["set-cookie"] << std::endl;
    // sList.printSesions();
    // cout << "Final Debug\n";
}

void readRequest(std::vector<Request> &req, int userId, char buffer[], size_t len, Config con, SessionsList &sList)
{
    size_t pos;
    std::string tes;
    req[userId].fullLine.append(buffer, len);
    req[userId].bodyPath = BODY_FILE_PATH + std::string("client") + toStr(userId + 1);
    while (1)
    {
        cout << "id = " <<  userId << std::endl;
        // cout << "--->" << req[userId].fullLine.length() << std::endl;
        if (req[userId].method.empty())
            getRequestLine(req[userId]);
        if (req[userId].method.empty()) // if no /r/n break 
        {
            if (req[userId].status == 400)
                req[userId].clientState = CLIENT_ON;
            break ;
        }
        else
        {
            pos = req[userId].fullLine.find("\r\n");
            tes = req[userId].fullLine;
            req[userId].reqState = ONE_ST;
        }
        // size_t pos = req[userId].fullLine.find("\r\n\r\n");
        // if (pos == std::string::npos)
        // {
        //     break;
        // }
        if (req[userId].getEndHeaders == false)
            getHeaders(req[userId]);
        if (req[userId].getEndHeaders == false)
            getHeaders(req[userId]);
        else
            req[userId].reqState = SECOND_ST;
        if (req[userId].method == "POST" && req[userId].getEndFile == false && req[userId].bodyType == FIXED)
            getFixedBody(req[userId], con);
        if (req[userId].lenRead == len)
        {
            // cout << tes << "-->" << tes.length() << std::endl;
            if ((req[userId].status == 400) || (req[userId].getEndFile) || (req[userId].method != "POST" && pos != std::string::npos && tes.length() == 2) )
                req[userId].clientState = CLIENT_ON;
            req[userId].lenRead = 0;    
            break ;
        }    
        // else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        //     getChunkedBody(req);
        if ((req[userId].getEndFile) /*|| (req[userId].method != "POST" && !req[userId].method.empty())*/)
            req[userId].finFlag = true;

        if (req[userId].finFlag)
        {
            cout << "SFSFSFSF\n";
            req[userId].reqState = SECOND_ST;
            req[userId].clientState = CLIENT_ON;
            break;
        }
    }
    cout << req[userId].getEndHeaders << std::endl;
    if (req[userId].getEndHeaders)
        generateCookies(req[userId], sList);
    return;
}
