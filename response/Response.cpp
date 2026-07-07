#include "../server/server.hpp"

void defaultErrorPage(playloadData &data)
{
    std::stringstream ss;
    ss <<
        "<html>\n"
        "<body>\n"
        "<br>\n"
        "<hr>\n";
    ss << "<h1 style=\"text-align: center;\">" << data.statusCode << ' ' << data.table[data.statusCode] << "</h1>\n";
    ss <<
        "<hr>\n"
        "</body>\n"
        "</html>\n";
    data.errorBody = ss.str();
    data.bodySize = data.errorBody.length();
    data.contentType = data.mime["html"];
}

void loadExtensions(playloadData &data)
{
    data.mime["html"] = "text/html";
    data.mime["css"] = "text/css";
    data.mime["xml"] = "text/xml";
    data.mime["txt"] = "text/plain";
    data.mime["plain"] = "text/plain";

    data.mime["png"] = "image/png";
    data.mime["jpeg"] = "image/jpeg";
    data.mime["jpg"] = "image/jpeg";
    data.mime["gif"] = "image/gif";
    data.mime["webp"] = "image/webp";

    data.mime["js"] = "application/javascript";
    data.mime["json"] = "application/json";
    data.mime["doc"] = "application/msword";
    data.mime["pdf"] = "application/pdf";
    data.mime["rar"] = "application/x-rar-compressed";
    data.mime["zip"] = "application/zip";

    data.mime["mp3"] = "audio/mpeg";
    data.mime["ogg"] = "audio/ogg";

    data.mime["mp4"] = "video/mp4";
    data.mime["webm"] = "video/webm";
    data.mime["mov"] = "video/quicktime";
}

void loadStatusCode(playloadData &data)
{
    data.table[200] = "OK";
    data.table[201] = "Created";
    data.table[301] = "Moved Permanently";
    data.table[302] = "Moved Temporarily";
    data.table[400] = "Bad Request";
    data.table[403] = "Forbidden";
    data.table[404] = "Not Found";
    data.table[405] = "Method Not Allowed";
    data.table[500] = "Internal Server Error";
    data.table[501] = "Not Implemented";
    data.table[502] = "Bad Gateway";
}

void getContentType(playloadData &data, std::string path)
{
    std::string extension;
    size_t pos = path.rfind('.');
    if (data.contentType.empty())
        data.contentType = "application/octet-stream";
    if (pos != std::string::npos)
    {
        extension = path.substr(pos + 1);
        if (data.mime.count(extension))
            data.contentType = data.mime[extension];
    }
}

bool deleteHandler(Record &log)
{
    struct stat info;

    if (access(log.data.realPath.c_str(), F_OK) == -1)
    {
        log.data.statusCode = 404;
        return true;
    }
    else if (stat(log.data.realPath.c_str(), &info) == -1)
    {
        log.data.statusCode = 500;
        return true;
    }
    else if (S_ISDIR(info.st_mode))
    {
        log.data.statusCode = 403;
        return true;
    }
    else if (std::remove(log.data.realPath.c_str()))
    {
        log.data.statusCode = 500;
        return true;
    }
    log.buffer = "File deleted successfully!\n";
    log.data.bodySize = log.buffer.length();
    log.data.contentType = log.data.mime["txt"];
    log.status = CLOSED_RS;
    return false;
}

std::string getRandName()
{
    static size_t count;
    std::stringstream ss;
    time_t tStamp = std::time(NULL);
    struct tm *currentTime = std::localtime(&tStamp);
    char arr[9];
    std::strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    ss << count++;
    randName += '_';
    randName += ss.str();
    return randName;
}

bool postBody(Record &log)
{
    char buff[RDWR_SIZE];
    int bytes = read(log.fileFd, buff, sizeof(buff));
    if (bytes == -1)
    {
        log.data.statusCode = 500;
        return true;
    }
    if ((write(log.fd, buff, bytes)) == -1)
    {
        log.data.statusCode = 500;
        return true;
    }
    if (bytes == 0)
    {
        log.status = CLOSED_RS;
        log.method = NONE_RS;
    }
    else
    {
        log.status = ACTIVE_RS;
        log.method = POST_RS;
    }
    return false;
}

bool postHandler(Record &log, Request &req)
{
    struct stat info;
    if (!log.data.uploadOn)
    {
        log.data.statusCode = 403;
        return true;
    }
    if (stat(log.data.realPath.c_str(), &info) != -1)
    {
        if (S_ISDIR(info.st_mode))
        {
            std::string randName = getRandName();
            std::string tmpStr = log.data.realPath + '/' + randName + ".txt";
            log.fd = open(tmpStr.c_str(), O_WRONLY | O_CREAT, 0644);
            if (log.fd == -1)
            {
                log.data.statusCode = 500;
                return true;
            }
            log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
            if (log.fileFd == -1)
            {
                log.data.statusCode = 500;
                return true;
            }
            if (postBody(log))
                return true;
            log.data.contentType = log.data.mime["txt"];
        }
        else
        {
            log.data.statusCode = 403;
            return true;
        }
    }
    else
    {
        log.fd = open(log.data.realPath.c_str(), O_WRONLY | O_CREAT, 0644);
        if (log.fd == -1)
        {
            log.data.statusCode = 500;
            return true;
        }
        log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (log.fileFd == -1)
        {
            log.data.statusCode = 500;
            return true;
        }
        if (postBody(log))
            return true;
    }
    log.data.statusCode = 201;
    log.buffer = "File uploaded\n";
    log.data.bodySize = log.buffer.length();
    log.data.contentType = log.data.mime["txt"];
    return false;
}

bool getFile(Record &log)
{
    char buffer[RDWR_SIZE];
    int bytes = 0;
    bytes = read(log.fd, buffer, sizeof(buffer));
    if (bytes == -1)
    {
        log.data.statusCode = 500;
        if (log.method == GET_LIST_RS)
            std::remove(log.fileName.c_str());
        return true;
    }
    log.buffer.append(buffer, bytes);
    if (bytes == 0)
    {
        log.status = CLOSED_RS;
        if (log.method == GET_LIST_RS)
            std::remove(log.fileName.c_str());
    }
    else if (log.status != ACTIVE_RS)
    {
        log.status = ACTIVE_RS;
        log.method = GET_RS;
    }
    return false;
}

std::string fileNameHandler(std::string tmp)
{
    unsigned char c;
    std::string fileName;
    char s[17] = "0123456789ABCDEF";
    for (size_t i = 0; i < tmp.length(); i++)
    {
        if (!isalpha(tmp[i]) && !isdigit(tmp[i]) &&
            tmp[i] != '.' && tmp[i] != '-' && tmp[i] != '_' && tmp[i] != '~')
        {
            c = tmp[i];
            fileName += '%';
            fileName += s[c / 16];
            fileName += s[c % 16];
            continue ;
        }
        fileName += tmp[i];
    }
    return fileName;
}

std::string cleanPath(std::string fullPath)
{
    std::stringstream ssPath(fullPath);
    std::vector<std::string> realNames;
    std::string tmp;
    std::string path;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "")
            continue;
        else
            realNames.push_back(tmp);
    }

    for (size_t i = 0; i < realNames.size(); i++)
    {
        path += '/' + realNames[i];
    }
    return path;
}

std::string handleAngelBrackets(std::string tmp)
{
    std::string retTmp;
    for (size_t i = 0; i < tmp.length(); i++)
    {
        if (tmp[i] == '<')
        {
            retTmp += "&lt;";
            continue;
        }
        else if (tmp[i] == '>')
        {
            retTmp += "&gt;";
            continue;
        }
        retTmp += tmp[i];
    }
    return retTmp;
}

bool postFilesList(Record &log)
{
    struct dirent *rdir;
    std::string tmp;
    std::string buff;
    int buffLength = 0;
    std::string tmpPath = log.data.location + log.data.uri;

    if (tmpPath[tmpPath.length() - 1] == '/')
        tmpPath.resize(tmpPath.length() - 1);
    tmpPath = cleanPath(tmpPath);
    while ((rdir = readdir(log.dir)))
    {	
        tmp = rdir->d_name;
        if (tmp == ".")
            continue;
        buff = "<a href=\"" + tmpPath + '/' + fileNameHandler(tmp) + "\">";
        if (tmp == "..")
            tmp = "~/";
        buff += handleAngelBrackets(tmp) + "</a><br>\n";
        buffLength += buff.length();
        if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
            return true;
        if (buffLength > RDWR_SIZE)
        {
            log.status = ACTIVE_RS;
            log.method = POST_LIST_RS;
            return false;
        }
    }
    buff =
        "</body>\n"
        "</html>\n";
    if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
        return true;
    log.method = GET_LIST_RS;
    struct stat info;
    if (stat(log.fileName.c_str(), &info) == -1)
        return true;
    log.data.bodySize = info.st_size;
    log.fd = open(log.fileName.c_str(), O_RDONLY);
    if (log.fd == -1)
        return true;
    return false;
}

bool autoIndex(Record &log, playloadData &data)
{
    log.fileName = LIST_FILE_PATH;
    log.fileName += "bodyList_" + getRandName();
    log.fileFd = open(log.fileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log.fileFd == -1)
        return true;
    log.dir = opendir(data.realPath.c_str());
    if (!log.dir)
    {
        std::remove(log.fileName.c_str());
        return true;
    }
    data.contentType = data.mime["html"];
    std::string buff;
    buff =
        "<html>\n"
        "<body>\n"
        "<br><hr><hr>"
        "<h1>Directory Listing</h1>\n";
    if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
    {
        std::remove(log.fileName.c_str());
        return true;
    }
    if (postFilesList(log))
    {
        std::remove(log.fileName.c_str());
        return true;
    }
    if (log.method == GET_LIST_RS)
    {
        if (getFile(log))
            return true;
    }
    return false;
}

bool tryIndex(Record &log, playloadData &data)
{
    for (size_t i = 0; i < data.index.size(); i++)
    {
        std::string pathIndex = data.realPath + '/' + data.index[i];
        if (access(pathIndex.c_str(), F_OK) == -1)
            continue;
        struct stat info;
        if (stat(pathIndex.c_str(), &info) == -1)
        {
            data.statusCode = 500;
            return true;
        }
        if (S_ISDIR(info.st_mode))
            continue;
        else if (access(pathIndex.c_str(), R_OK) == -1)
        {
            data.statusCode = 403;
            return true;
        }
        log.fd = open(pathIndex.c_str(), O_RDONLY);
        if (log.fd == -1)
        {
            data.statusCode = 500;
            return true;
        }
        data.bodySize = info.st_size;
        getContentType(data, pathIndex);
        break;
    }
    if (log.fd != -1)
    {
        if (getFile(log))
            return true;
    }
    else
        return false;
    return true;
}

int validateFile(Record &log, playloadData &data)
{
    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.statusCode = 404;
        return 1;
    }
    else if (access(data.realPath.c_str(), R_OK) == -1)
    {
        data.statusCode = 403;
        return 1;
    }
    struct stat info;
    if (stat(data.realPath.c_str(), &info) == -1)
    {
        data.statusCode = 500;
        return 1;
    }
    if (S_ISDIR(info.st_mode))
    {
        if (tryIndex(log, data))
            return 1;
        if (data.autoIndex)
        {
            if (autoIndex(log, data))
            {
                data.statusCode = 500;
                return 1;
            }
        }
        else
        {
            data.statusCode = 403;
        }
        return 1;
    }
    data.bodySize = info.st_size;
    getContentType(data, data.realPath);
    return 0;
}

bool getHandler(Record &log, playloadData &data)
{
    int check;
    check = validateFile(log, data);
    if (data.statusCode != 200)
        return true;
    if (check)
        return false;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.statusCode = 500;
        return true;
    }
    if (getFile(log))
        return true;
    return false;
}

void findLongestMatch(playloadData &data, Config &conf, std::string reqPath)
{
    std::map<int, std::string> locs;
    if (reqPath.empty())
        reqPath = "/";
    for (size_t i = 0; i < conf.location.size(); i++)
    {
        std::string name = conf.location[i].location_name;
        size_t f = reqPath.find(name);
        if (f == 0
            && (reqPath.length() == name.length()
            || reqPath[name.length()] == '/' || name == "/"))
            locs[conf.location[i].location_name.length()] = conf.location[i].location_name;
    }
    if (locs.empty())
        data.uri = reqPath;
    else
    {
        data.location = locs.rbegin()->second;
        data.uri = reqPath.substr(data.location.length());
    }
    if (data.uri[0] != '/')
        data.uri.insert(0, "/");
}

void getImportantData(Config &conf, playloadData &data)
{
    if (data.location.empty())
    {
        data.location = "/";
        data.methods = conf.methods;
        data.root = conf.root;
        data.index = conf.index;
        data.upload = conf.uploadPath;
        data.autoIndex = conf.autoIndex;//?AUTOF
        data.errorPages = conf.error_page;
        data.retPath = conf.retPath;
        data.retCode = conf.retCode;
        data.cgiExt = conf.cgiExt;
        data.cgiPath = conf.cgiPath;
        return ;
    }
    for (size_t i = 0; i < conf.location.size(); i++)
    {
        if (conf.location[i].location_name == data.location)
        {
            data.errorPages = conf.location[i].conf.error_page;
            data.methods = conf.location[i].conf.methods;
            data.index = conf.location[i].conf.index;
            data.root = conf.location[i].conf.root;
            data.upload = conf.location[i].conf.uploadPath;
            data.autoIndex = conf.location[i].conf.autoIndex;
            data.retPath = conf.location[i].conf.retPath;
            data.retCode = conf.location[i].conf.retCode;
            data.cgiExt = conf.location[i].conf.cgiExt;
            data.cgiPath = conf.location[i].conf.cgiPath;
            break;
        }
    }
}

int checkErrorFile(playloadData &data)
{
    std::string file = data.root;
    if (file[file.length() - 1] != '/')
        file += '/';
    std::string page = data.errorPages[data.statusCode];
    if (page.empty())
        return 1;
    if (page[0] == '/')
        page = page.substr(1);
    file += page;
    struct stat info;
    if (stat(file.c_str(), &info) == -1)
        return 1;
    if (S_ISDIR(info.st_mode))
        return 1;
    
    if (info.st_size > 64 * 1024)
        return 1;
    int fd = open(file.c_str(), O_RDONLY);
    if (fd == -1)
        return 1;
    char buff[RDWR_SIZE];
    int bytes = 0;
    while ((bytes = read(fd, buff, sizeof(buff) - 1)) > 0)
        data.errorBody.append(buff, bytes);
    close(fd);
    if (bytes < 0)
        return 1;
    data.bodySize = info.st_size;
    data.contentType = data.mime["txt"];
    getContentType(data, file);
    return 0;
}

void chooseErrorHandler(playloadData &data)
{
    if (data.errorPages.empty())
        defaultErrorPage(data);
    else if (checkErrorFile(data))
        defaultErrorPage(data);
}

int checkCgi(playloadData &data, std::string realNames, std::string &reqPath)
{
    for (size_t i = 0; i < data.cgiExt.size(); i++)
    {
        size_t k = data.realPath.rfind(data.cgiExt[i]);
        if (k != std::string::npos 
            && (k + data.cgiExt[i].size() == data.realPath.size() || data.realPath[k + data.cgiExt[i].size()] == '?'))
        {
            data.cgiInfo = data.realPath.substr(k + data.cgiExt[i].size());
            data.cgiScriptFile = data.realPath.substr(0, k + data.cgiExt[i].size());
            data.cgiInfo += reqPath.substr(reqPath.find(realNames) + realNames.length());
            data.cgiFile = cleanPath(reqPath.substr(0, reqPath.length() - data.cgiInfo.length()));
            data.cgiExecutePath = data.cgiPath[i];
            return 1;
        }
    }
    return 0;
}

int authenCheck(playloadData &data, Request &req)
{
    if (req.method != "GET" && req.method != "POST")
        return 0;
    if (data.loginToken == req.path && req.isLoggedCookie)
    {
        if (req.method != "GET")
            return 0;
        data.retPath = data.redirectTokens[0];
        data.retValid = true;
        data.retCode = 302;
        return 1;
    }
    else
    {
        for (size_t i = 0; i < data.redirectTokens.size(); i++)
        {
            if (data.redirectTokens[i] == req.path && req.isLoggedCookie == false)
            {
                data.retPath = data.loginToken;
                data.retValid = true;
                data.retCode = 302;
                return 1;
            }
            else if (data.redirectTokens[i] == req.path && req.isLoggedCookie)
            {
                if (req.method == "POST")
                {
                    data.retPath = req.path;
                    data.retValid = true;
                    data.retCode = 302;
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

bool resolvePath(playloadData &data, Request &req)
{
    struct stat info;
    std::stringstream ssPath(data.root + data.uri);
    std::vector<std::string> realNames;
    std::string tmp;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "" || tmp == ".")
            continue;
        else if (tmp == "..")
        {
            data.statusCode = 403;
            return true;
        }
        else
            realNames.push_back(tmp);
    }
    if (authenCheck(data, req))
        return false;
    for (size_t i = 0; i < realNames.size(); i++)
    {
        data.realPath += '/' + realNames[i];
        if (!checkCgi(data, realNames[i], req.path))
            continue;
        if (stat(data.cgiScriptFile.c_str(), &info) != -1)
        {
            if (S_ISDIR(info.st_mode))
                continue;
        }
        data.isCgi = true;
        return false;
    }

    if (realNames.empty())
        data.realPath = "/";
    data.uploadOn = false;
    if (req.method != "POST" || data.upload.empty())
        return false;
        
    size_t find = data.uri.find(data.upload);
    if (find == 0
        && (data.uri.length() == data.upload.length() || data.uri[data.upload.length()] == '/'))
        data.uploadOn = true;
    return false;
}

bool responseHandler(Webserv &Web, Record &log, Request &req, Config &conf, size_t &i, SessionsList &sList)
{
    (void) sList;//! remove it if no need
    log.data.retValid = false;
    log.data.isCgi = false;
    log.data.statusCode = req.status;
    if (req.headers["connection"] == "Keep-Alive")
        log.data.connection = "keep-alive";
    getImportantData(conf, log.data);
    if (log.data.statusCode != 200)
        return true;
    if (!log.data.methods[req.method])
    {
        log.data.statusCode = 405;
        return true;
    }
    if (!log.data.retPath.empty())
    {
        if (req.method == "POST")
        {
            if (access(req.bodyPath.c_str(), F_OK) != -1)
                std::remove(req.bodyPath.c_str());
        }
        if (log.data.retCode != 301 && log.data.retCode != 302)
        {
            log.data.statusCode = 501;
            return true;
        }
        log.data.retValid = true;
        return false;
    }
    if (resolvePath(log.data, req))
        return true;
    if (log.data.isCgi)
    {
        if (cgiHandler(Web, log, req, conf.listen_Port,i))
        {
            log.cgiData.cgiValid = false;
            return true;
        }
        return false;
    }
    if (req.method == "GET")
    {
        if (getHandler(log, log.data))
            return true;
    }
    else if (req.method == "POST")
    {
        if (postHandler(log, req))
        {
            if (access(req.bodyPath.c_str(), F_OK) != -1)
                std::remove(req.bodyPath.c_str());
            log.method = NONE_RS;
            return true;
        }
        if (log.status == CLOSED_RS)
        {
            if (access(req.bodyPath.c_str(), F_OK) != -1)
                std::remove(req.bodyPath.c_str());
        }
    }
    else if (req.method == "DELETE")
    {
        if (deleteHandler(log))
            return true;
    }
    return false;
}

void generateCgiResponse(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Request &req)
{
    std::stringstream res;
    res << "HTTP/1.0 " << data.statusCode << ' ' << data.table[data.statusCode] + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    std::map<std::string, std::string>::iterator iter;
    // int ok = 0;
    for (iter = cgiData.cgHeaders.begin(); iter != cgiData.cgHeaders.end(); iter++)
    {
        if (tolowerStr(iter->first) == "status" || tolowerStr(iter->first) == "set-cookie"
            || tolowerStr(iter->first) == "connection" || tolowerStr(iter->first) == "content-length")
            continue;
        res << iter->first << ": " << iter->second << "\r\n";
    }
    res << "Content-Length: " << data.bodySize << "\r\n";
    if (req.firstTime == true)
    {
        res << "Set-Cookie: session_id=" << req.cookies["session_id"] << "; path=/" << "\r\n";
        req.firstTime = false;
    }
    res << "\r\n";
    ret_resp = res.str();
}

void generateRetResponse(playloadData &data, std::string &ret_resp, Request &req)
{
    std::stringstream res;
    res << "HTTP/1.0 " << data.retCode << " " << data.table[data.retCode] << "\r\n";
    res << "Location: " << data.retPath << "\r\n";
    if (req.firstTime == true)
    {
        res << "Set-Cookie: session_id=" << req.cookies["session_id"] << "; path=/" << "\r\n";
        req.firstTime = false;
    }
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Length: 0\r\n";
    res << "\r\n";
    ret_resp = res.str();
    return;
}

void responseGenerator(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Record &log, Request &req)
{
    if (data.statusCode != 200 && data.statusCode != 201)
        data.connection = "close";
    if (tolowerStr(data.connection) == "keep-alive")
        log.clientState = ALIVE;
    if (cgiData.cgiValid)
    {
        generateCgiResponse(cgiData, data, ret_resp, req);
        return;
    }
    if (data.retValid)
    {
        generateRetResponse(data, ret_resp, req);
        log.status = CLOSED_RS;
        return;
    }
    std::stringstream res;
    res << "HTTP/1.0 " << data.statusCode << ' ' << data.table[data.statusCode] + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    if (req.firstTime == true)
    {
        res << "Set-Cookie: session_id=" << req.cookies["session_id"] << "; path=/" << "\r\n";
        req.firstTime = false;
    }
    res << "\r\n";
    ret_resp = res.str();
}

std::string respond(Webserv &Web, Record &log, Config &conf, Request &req, size_t &indexClnt, SessionsList &sList)
{
    std::string ret_resp;
    t_state pastStatus = log.status;
    t_state pastMethod = log.method;
    if (log.status == NEW_RS)
    {
        log.data.bodySize = 0;
        loadExtensions(log.data);
        loadStatusCode(log.data);
        log.data.connection = "Close";
        if (responseHandler(Web, log, req, conf,indexClnt, sList))
        {
            chooseErrorHandler(log.data);
            log.method = NONE_RS;
            log.status = ERROR_RS;
        }
    }
    else if (log.status == ACTIVE_RS || log.status == CGI_ACTIVE_RS)
    {
        if (log.method == CGI_RDWR)
        {
            if (cgiProcess(Web, log, req,indexClnt))
            {
                chooseErrorHandler(log.data);
                log.status = ERROR_RS;
                log.method = NONE_RS;
            }
            if (log.status == ERROR_RS || log.status == CLOSED_RS)
            {
                if (access(log.cgiData.cgBodyFileName.c_str(), F_OK) != -1)
                    std::remove(log.cgiData.cgBodyFileName.c_str());
            }
        }
        if (log.method == POST_LIST_RS)
        {
            if (postFilesList(log))
            {
                std::remove(log.fileName.c_str());
                log.data.statusCode = 500;
                log.status = ERROR_RS;
                log.method = NONE_RS;
            }
        }
        if (log.method == GET_RS || log.method == GET_LIST_RS)
        {
            if (getFile(log))
                log.status = ERROR_RS;
            if (log.status == ERROR_RS || log.status == CLOSED_RS)
            {
                if (access(log.cgiData.cgBodyFileName.c_str(), F_OK) != -1)
                    std::remove(log.cgiData.cgBodyFileName.c_str());
            }
        }
        else if (log.method == POST_RS)
        {
            if (postBody(log))
            {
                chooseErrorHandler(log.data);
                log.method = NONE_RS;
                log.status = ERROR_RS;
            }
            if (log.status == ERROR_RS || log.status == CLOSED_RS)
            {
                if (access(req.bodyPath.c_str(), F_OK) != -1)
                    std::remove(req.bodyPath.c_str());
            }
        }
    }
    if (log.method == POST_LIST_RS || log.method == POST_RS || log.status == CGI_ACTIVE_RS)
        return "";

    if (pastStatus == NEW_RS
        || pastMethod == POST_RS
        || pastMethod == POST_LIST_RS
        || pastStatus == CGI_ACTIVE_RS)
    {
        responseGenerator(log.cgiData, log.data, ret_resp, log, req);
        if (log.status == ERROR_RS)
            ret_resp += log.data.errorBody;
        else
            ret_resp += log.buffer;
    }
    else if (log.status != ERROR_RS)
        ret_resp = log.buffer;
    log.buffer.clear();
    return (ret_resp);
}
