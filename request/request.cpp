#include "../server/server.hpp"

Request::Request() : bodyLen(0), lenRead(0), countConLen(0), bodyFd(-1), status(200), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_OF)
{
    bodyType = 0;
    getMove = 0;
    countLenHead = 0;
    recLen = 0;
    isLoggedCookie = false;
};

void Request::clean() // add youssef
{
    if (access(bodyPath.c_str(), F_OK) != -1)
        std::remove(bodyPath.c_str());
}

void findLongestMatchLocation(playloadData &data, Config &conf, std::string reqPath)
{
    std::map<int, std::string> locs;
    if (reqPath.empty())
        reqPath = "/";
    for (size_t i = 0; i < conf.location.size(); i++)
    {
        std::string name = conf.location[i].location_name;
        size_t f = reqPath.find(name);
        if (f == 0 && (reqPath.length() == name.length() || reqPath[name.length()] == '/' || name == "/"))
            locs[conf.location[i].location_name.length()] = conf.location[i].location_name;
    }
    if (locs.empty())
    {
        data.uri = reqPath.substr(1);
        if (data.uri[0] != '/')
            data.uri.insert(0, "/");
        return;
    }
    else
        data.location = locs.rbegin()->second;
    data.uri = reqPath.substr(data.location.length());
    if (data.uri[0] != '/')
        data.uri.insert(0, "/");
}

bool checkDigits(Request &req, std::string str)
{
    std::stringstream iss(str);
    size_t value;
    iss >> value;
    
    if (!str.empty() && str.at(0) != '-' && !iss.fail() && iss.eof())
    {
        req.contentLength = value;
        return (true);
    }
    return (false);
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

void Request::generateRandomName(std::string value)
{
    static int i = 1;
    if (access(value.c_str(), X_OK))
        this->bodyPath = value + toStr(i + 1);
    i++;
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
        if (!isprint(static_cast<unsigned char>(str[i])))
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

bool checkMethod(std::string &method)
{
    return (!(method != "POST" && method != "GET" && method != "DELETE"));
}

bool checkPath(std::string &path)
{
    std::stringstream iss(path);
    std::string splitedPath;
    std::string tmp;

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

void getRequestLine(Request &req, playloadData &pdata, Config &conf)
{
    size_t pos = req.fullLine.find("\r\n");
    if (pos == std::string::npos)
        return ;
    fillCheck(req, req.fullLine.substr(0, pos));
    req.lenRead += req.fullLine.substr(0, pos + 2).length();
    req.fullLine.erase(0, pos + 2);
    req.getMove = 1;
    if (req.status == 200)
    {
        pdata.loginToken = "/portf/html/login.html";//tmp
        pdata.redirectTokens.push_back("/portf/html/index.html");//tmp
        findLongestMatchLocation(pdata, conf, req.path);
        for (size_t i = 0; i < conf.location.size(); i++)
        {
            if (conf.location[i].location_name == pdata.location)
            {
                req.maxBodySize = conf.location[i].conf.client_max_body_size ;
                // pdata.loginToken = conf.location[i].conf.loginToken;
                // pdata.redirectTokens = conf.location[i].conf.redirectTokens;
            }
        }
    }
    
}

void getHeaders(Request &req)
{
    size_t endLine = req.fullLine.find("\r\n");
    if (endLine == std::string::npos)
    {
        if (req.getMove == 2)
            req.getMove = 3;
        return ;
    }
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
        else if (req.method != "GET" && req.method != "DELETE")
            return (returnedBadError(req));
        // if (req.method == "POST" && req.contentLength > req.maxBodySize)
        //     return (returnedBadError(req));
        return ;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = tolowerStr(line.substr(0, colon));
        std::string value = line.substr(colon + 1);
        if (key.length() >= MAX_LEN_HEADER_FEILD || value.length() >= MAX_LEN_HEADER_FEILD)
            return (returnedBadError(req));
        if (!isPrintable(key) || !isHoldSpace(key) || key.empty())
            return (returnedBadError(req));
        value = trim(value);
        if (req.headers.find(key) != req.headers.end() && key == "content-length")
            return (returnedBadError(req));
        req.headers[key] = value;
        if (req.headers.find("content-length") != req.headers.end() && !checkDigits(req, req.headers["content-length"]))
            return (returnedBadError(req));
    }
    else
        return (returnedBadError(req));
    req.lenRead += req.fullLine.substr(0, endLine + 2).length();
    req.countLenHead += req.fullLine.substr(0, endLine + 2).length();    
    req.fullLine.erase(0, endLine + 2);
    // cout << "O => " << req.countLenHead << std::endl;
    if (req.countLenHead >= MAX_LEN_HEADERS_FEILD)
        return (returnedBadError(req));
    return ;
}


void getChunkedBody(Request &req)
{
    size_t endTxt;
    size_t endNum;
    if (req.bodyFd == -1)
    {
        req.bodyLen = 0;
        req.generateRandomName(BODY_FILE_PATH + std::string("client"));
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (req.bodyFd == -1)
        {
            req.getEndFile = true;
            return (returnedInternalError(req));
        }
    }
    endNum = req.fullLine.find("\r\n");
    if (endNum != std::string::npos)
        endTxt = req.fullLine.find("\r\n", endNum + 2);
    while (endNum != std::string::npos && endTxt != std::string::npos)
    {
        char *end = NULL;
        std::string chunkNum  = req.fullLine.substr(0, endNum);
        std::string chunkData = req.fullLine.substr(endNum + 2, endTxt - endNum - 2);
        req.lenRead += req.fullLine.substr(0, endTxt + 2).length();
        double chunkSize = std::strtol(chunkNum.c_str(), &end, 16);
        // cout << "3amro -> " << chunkSize << std::endl;
        req.bodyLen += chunkSize;
        if (errno == ERANGE || *end || chunkSize < 0 || req.bodyLen > req.maxBodySize)
        {
            req.status = 400;
            close(req.bodyFd);
            return ;
        }
        if (chunkSize == 0 && chunkData.empty())
        {
            // cout << "BOOYAH\n";
            write(req.bodyFd, chunkData.c_str(), chunkSize);
            req.fullLine.erase(0, endTxt + 2);
            req.getEndFile = true;
            close(req.bodyFd);
            return ;
        }
        if (chunkData.length() < (size_t)chunkSize)
        {
            req.status = 400;
            close(req.bodyFd);
            return ;
        }
        write(req.bodyFd, chunkData.c_str(), chunkSize);
        req.fullLine.erase(0, endTxt + 2);
        endNum = req.fullLine.find("\r\n");
        if (endNum != std::string::npos)
            endTxt = req.fullLine.find("\r\n", endNum + 2);
    }
    if (req.getMove == 4)
        req.getMove = 3;
}

void getFixedBody(Request &req)
{
    req.lenRead += req.fullLine.length();
    if (req.bodyFd == -1)
    {
        req.bodyLen = 0;
        req.generateRandomName(BODY_FILE_PATH + std::string("client"));
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
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
    // cout << " --------------> " << email << std::endl;
    return (email);
}

void    generateCookies(Request &req, SessionsList &sList, playloadData &pdata)
{
    sList.generateSession(req);
    // std::cout << pdata.redirectTokens[0] << ' ' << req.path << "\n";
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
            }//case0
            else if (bytes > 0)
            {
                std::string email = getEmail(buff);
                sList.storeInputs(req, email);
                close(fd);
            }
        }
    }
}

void readRequest(Request &req, char buffer[], size_t len, Config con, SessionsList &sList, playloadData &pdata)
{
    size_t pos;
    std::string tes;
    req.fullLine.append(buffer, len);
    req.recLen += len;
    if (req.getMove == 3)
        req.getMove = 0;
    // cout << "-----> " << req.fullLine <<  "\n";
    while (1)
    {
        if (req.method.empty())
            getRequestLine(req, pdata, con);
        if (req.method.empty())
        {
            if (req.status != 200 )
                req.clientState = CLIENT_ON;
            break ;
        }
        else
        {
            pos = req.fullLine.find("\r\n");
            tes = req.fullLine;
        }

        if (req.getEndHeaders == false)
        {
            getHeaders(req);
            if (req.status == 500 || req.status == 400 || req.status == 501)
                req.clientState = CLIENT_ON ;
            if (req.getMove == 3)
                break ;
            req.getMove = 2; // This is the change in header
        }
        if (req.method == "POST" && req.getEndFile == false && req.bodyType == FIXED)
            getFixedBody(req);
        else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        {
            getChunkedBody(req);
            if (req.getMove == 3)
                break ;
        }
        req.getMove = 4;
        if (req.lenRead == req.recLen)
        {
            if ((req.getEndFile) || (req.status == 500) || (req.status == 400) || (req.status == 501))
                req.clientState = CLIENT_ON;
            if (pos != std::string::npos && tes.length() > 2)
                tes.erase(0, pos + 2);
            if (req.method == "GET" || req.method == "DELETE")
            {
                if ((req.method != "POST" && pos != std::string::npos && tes.length() == 2))
                    req.clientState = CLIENT_ON;
            }
            req.lenRead = 0;
            req.recLen  = 0;
            break ;
        }
        
        if ((req.getEndFile))  /*|| (req.method != "POST" && !req.method.empty())*/
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
