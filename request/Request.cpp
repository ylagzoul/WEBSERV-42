#include "../server/server.hpp"

Request::Request() : bodyLen(0), lenRead(0), countConLen(0), bodyFd(-1), status(200), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_OF)
{
    bodyType = 0;
    countLenHead = 0;
    recLen = 0;
    chunkSize = 0;
    isLoggedCookie = false;
    firstTime = false;
};

void Request::clean()
{
    if (access(bodyPath.c_str(), F_OK) != -1)
        std::remove(bodyPath.c_str());
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

std::string toStr(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
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

void    returnedBadError(Request &req)
{
    req.status = 400;
    req.finFlag = true;
}

void    returnedInternalError(Request &req)
{
    req.status = 500;
    req.finFlag = true;
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
        if (!isprint((unsigned char)(str[i])))
            return false;
    }
    return true;
}

int    count_space(std::string str)
{
    int count = 0;
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == ' ')
            count++;
        if (!isprint(str.at(i)))
            return (count = 5, count);
    }
    return (count);
}

bool isLfCr(const std::string str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str.at(i) == '\r' || str.at(i) == '\n')
            return false;
    }
    return (true);
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

    return (high * 16 + low);
}

bool checkPrecentEnc(std::string &pathSlice)
{
    std::string copyPath = pathSlice;
    pathSlice.clear();
    for (size_t i = 0; i < copyPath.length(); i++)
    {
        if (copyPath.at(i) == '%')
        {
            if (!(i + 2 < copyPath.size()))
                return (false);
            if (!isHex((char)copyPath.at(i+1)) || !isHex((char)copyPath.at(i+2)))
                return(false);
            char res = hexToChar(copyPath.at(i+1), copyPath.at(i+2));
            if ((unsigned char)res > 127)
                return false ;
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

bool checkMethod(std::string &method)
{
    return (!(method != "POST" && method != "GET" && method != "DELETE"));
}

bool checkPath(std::string &path)
{
    std::stringstream iss(path);
    std::string splitedPath;
    std::string tmp;
    bool isSlash = false;
    
    if (path.at(path.length() - 1) == '/')
        isSlash = true;
    if (path.find("%00") != std::string::npos)
        return (false);
    if (path.at(0) != '/' && path.at(0) != '%')
        return (false);
    else if (path.at(0) == '/' && path.length() == 1)
        return (true);
    while (getline(iss, splitedPath, '/'))
    {
        if (!splitedPath.empty())
        {
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
    if (isSlash)
        path += '/';
    return (true);
}

void fillCheck(Request &req, std::string reqLine)
{
    std::stringstream splitReq(reqLine);
    std::string slice;
    std::vector<std::string> array;

    if (count_space(reqLine) > 2)
        return (returnedBadError(req));
    while (splitReq >> slice)
        array.push_back(slice); 
    if (array.size() != 3)
        return (returnedBadError(req));
    if (array[2] != "HTTP/1.1" && array[2] != "HTTP/1.0")
        return (returnedBadError(req));
    req.method = array[0];
    req.path = array[1];
    req.httpVersion = array[2];
    if ( req.path.length() >= MAX_LEN_PATH || !checkPath(req.path))
        return (returnedBadError(req));
    if (!checkMethod(req.method))
    {
        req.status = 501;
        req.finFlag = true;
        return ;
    }
}

void getRequestLine(Request &req, PlayloadData &pdata, Config &conf)
{
    size_t pos = req.fullLine.find("\r\n");
    if (pos == std::string::npos)
        return ;
    fillCheck(req, req.fullLine.substr(0, pos));
    req.lenRead += req.fullLine.substr(0, pos + 2).length();
    req.fullLine.erase(0, pos + 2);
    if (req.status == 200)
    {
        findLongestMatch(pdata, conf, cleanPath(req.path));
        for (size_t i = 0; i < conf.location.size(); i++)
        {
            if (conf.location[i].location_name == pdata.location)
            {
                req.maxBodySize = conf.location[i].conf.client_max_body_size ;
                pdata.loginToken = conf.location[i].conf.loginToken;
                pdata.redirectTokens = conf.location[i].conf.redirectTokens;
            }
        }
        if (pdata.location.empty())
        {
            req.maxBodySize = conf.client_max_body_size;
            pdata.loginToken = conf.loginToken;
            pdata.redirectTokens = conf.redirectTokens;
        }
    }
    
}

bool getHeaders(Request &req)
{
    size_t endLine = req.fullLine.find("\r\n");
    if (endLine == std::string::npos)
        return (false);
    std::string line = req.fullLine.substr(0, endLine);
    if (line.empty())
    {
        req.getEndHeaders = true ;
        req.lenRead += req.fullLine.substr(0, endLine + 2).length();
        req.fullLine.erase(0, endLine + 2);
        std::map<std::string, std::string>::iterator it;
        it = req.headers.find("transfer-encoding");
        if (req.headers.find("host") != req.headers.end())
            req.host = req.headers["host"];
        if (req.headers.find("cookie") != req.headers.end())
            req.storeCookies(req.headers["cookie"]);
        if (it != req.headers.end() && it->second == "chunked")
            req.bodyType = CHUNKED;
        else if (req.headers.find("content-length") != req.headers.end())
            req.bodyType = FIXED;
        else if (req.method == "POST")
            return (returnedBadError(req), true);
        return (true);
    }
    size_t column = line.find(':');
    if (column != std::string::npos)
    {
        std::string key = tolowerStr(line.substr(0, column));
        std::string value = line.substr(column + 1);
        if (key.length() >= MAX_LEN_HEADER_FEILD || value.length() >= MAX_LEN_HEADER_FEILD)
            return (returnedBadError(req), true);
        if (!isPrintable(key) || !isHoldSpace(key) || key.empty() || !isLfCr(key) || !isLfCr(value))
            return (returnedBadError(req), true);
        value = trim(value);
        if (req.headers.find(key) != req.headers.end() && key == "content-length")
            return (returnedBadError(req), true);
        req.headers[key] = value;
        if (req.headers.find("content-length") != req.headers.end() && !checkDigits(req, req.headers["content-length"]))
            return (returnedBadError(req), true);
    }
    else
        return (returnedBadError(req), true);
    req.lenRead += req.fullLine.substr(0, endLine + 2).length();
    req.countLenHead += req.fullLine.substr(0, endLine + 2).length();    
    req.fullLine.erase(0, endLine + 2);
    if (req.countLenHead >= MAX_LEN_HEADERS_FEILD)
        return (returnedBadError(req), true);
    return (true);
}


bool getChunkedBody(Request &req)
{
    size_t endTxt = 0;
    size_t endNum = 0;
    if (req.bodyFd == -1)
    {
        req.bodyLen = 0;
        req.bodyPath = BODY_FILE_PATH + std::string("client_") + getRandName();
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR, 0644);
        if (req.bodyFd == -1)
        {
            req.getEndFile = true;
            return (returnedInternalError(req), true);
        }
    }   
    endNum = req.fullLine.find("\r\n");
    if (endNum != std::string::npos)
    {
        std::string tmp = req.fullLine.substr(0, endNum + 2);
        if (!checkDigits(req, tmp))
            return (close(req.bodyFd), returnedBadError(req), true);
        endTxt = req.fullLine.find("\r\n", endNum + 2);
    }
    
    while (endNum != std::string::npos && endTxt != std::string::npos)
    {
        std::string chunkData = req.fullLine.substr(endNum + 2, endTxt - endNum - 2);
        req.lenRead += req.fullLine.substr(0, endTxt + 2).length();
        req.bodyLen += req.chunkSize;
        if (req.bodyLen > req.maxBodySize)
            return (returnedBadError(req), true);
        if (req.chunkSize == 0 && chunkData.empty())
        {
            if (write(req.bodyFd, chunkData.c_str(), req.chunkSize) == -1)
                return (returnedInternalError(req), true);
            req.fullLine.erase(0, endTxt + 2);
            req.getEndFile = true;
            close(req.bodyFd);
            return (true);
        }
        if (chunkData.length() < (size_t)req.chunkSize)
        {
            req.status = 400;
            close(req.bodyFd);
            return (true);
        }
        if (write(req.bodyFd, chunkData.c_str(), req.chunkSize) == -1)
                return (returnedInternalError(req), true);
        req.fullLine.erase(0, endTxt + 2);
        endNum = req.fullLine.find("\r\n");
        if (endNum != std::string::npos)
        {
            std::string tmp = req.fullLine.substr(0, endNum + 2);
            if (!checkDigits(req, tmp))
                return (close(req.bodyFd), returnedBadError(req), true);
            endTxt = req.fullLine.find("\r\n", endNum + 2);
        }
    }
    return (false);
}

void getFixedBody(Request &req)
{
    req.lenRead += req.fullLine.length();
    if (req.bodyFd == -1)
    {
        req.bodyLen = 0;
        req.bodyPath = BODY_FILE_PATH + std::string("client_") + getRandName();
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR, 0644);
        if (req.bodyFd == -1)
        {
            req.getEndFile = true;
            return (returnedInternalError(req));
        }
    }
    req.bodyLen += req.fullLine.length();
    if (req.contentLength > req.maxBodySize)
    {
        req.getEndFile = true;
        return (returnedBadError(req));
    }
    if (req.bodyLen < req.contentLength)
    {
        if (req.bodyLen != 0)
        {
            if (write(req.bodyFd, req.fullLine.c_str(), req.fullLine.length()) == -1)
                return (returnedInternalError(req));
            req.fullLine.clear();
        }
        return;
    }
    int sizePa = (req.contentLength + req.fullLine.length()) - req.bodyLen;
    std::string d = req.fullLine.substr(0, sizePa);

    if (write(req.bodyFd, d.c_str(), sizePa) == -1)
        return (returnedInternalError(req));
    close(req.bodyFd);
    req.getEndFile = true;
}

std::string getEmail(char *buff)
{
    std::string tmp;
    if (buff)
        tmp = buff;
    std::stringstream ss(tmp);
    std::string email;
    std::getline(ss, email, '&');
    return (email);
}

void    generateCookies(Request &req, SessionsList &sList, PlayloadData &pdata)
{
    sList.generateSession(req);
    for (size_t i = 0; i < pdata.redirectTokens.size(); i++)
    {
        if (pdata.redirectTokens[i] == req.path && req.method == "POST")
        {
            char buff[SIZESR];
            int fd = open(req.bodyPath.c_str(), O_RDONLY);
            if (fd == -1)
            {
                req.status = 500;
                return ;
            }
            int bytes = read(fd, buff, SIZESR);
            if (bytes == -1)
            {
                req.status = 500;
                close(fd);
                return ;
            }
            else if (bytes > 0)
            {
                std::string email = getEmail(buff);
                sList.storeInputs(req, email);
                req.firstTime = true;
                close(fd);
            }
        }
    }
}

void readRequest(Request &req, char buffer[], size_t len, Config con, SessionsList &sList, PlayloadData &pdata)
{
    size_t pos;
    std::string tes;
    req.fullLine.append(buffer, len);
    req.recLen += len;
    while (1)
    {
        if (req.method.empty())
            getRequestLine(req, pdata, con);
        if (req.method.empty())
        {
            if (req.status != 200)
                req.clientState = CLIENT_ON;
            break ;
        }
        else
        {
            pos = req.fullLine.find("\r\n");
            tes = req.fullLine;
        }

        if (req.getEndHeaders == false && req.status == 200)
        {
            if (getHeaders(req) == false)
                break ;
            if (req.status == 400 || req.status == 501)
                req.clientState = CLIENT_ON ;
        }
        if (req.method == "POST" && req.getEndFile == false && req.bodyType == FIXED)
            getFixedBody(req);
        else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        {
            if (getChunkedBody(req) == false)
                break ;
        }
        if (req.lenRead == req.recLen)
        {
            if ((req.getEndFile) || (req.status == 500) || (req.status == 400) || (req.status == 501))
                req.clientState = CLIENT_ON;
            if (pos != std::string::npos && tes.length() > 2)
                tes.erase(0, pos + 2);
            if (req.method == "GET" || req.method == "DELETE")
            {
                if ((pos != std::string::npos && tes.length() == 2))
                    req.clientState = CLIENT_ON;
            }
            req.lenRead = 0;
            req.recLen  = 0;
            break ;
        }
        
        if ((req.getEndFile))
            req.finFlag = true;
        if (req.finFlag)
        {
            req.clientState = CLIENT_ON;
            break;
        }
    }
    if (req.getEndHeaders)
        generateCookies(req, sList, pdata);
    return;
}
