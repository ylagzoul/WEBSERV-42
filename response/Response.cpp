#include "../server/server.hpp"

void defaultErrorPage(playloadData &data)
{
    std::stringstream ss;
    ss <<
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>Error page</title>\n"
        "</head>\n"
        "<body>\n";
    ss << "<h1 style=\"text-align: center;\">" << data.statusCode << ' ' << data.table[data.statusCode] << "</h1>\n";
    ss <<
        "<hr>"
        "</body>\n"
        "</html>\n";
    data.bodyStr = ss.str();
    data.bodySize = data.bodyStr.length();
}

int readyChecks(Request &req, playloadData &data)
{
    data.statusCode = req.status;
    if (req.status != 200)
    {
        if (req.status)
        // data.statusCode = "400";
        return 1;
    }
    // else if (req.status == 500)
    // {
        // data.statusCode = "500";
        // return 1;
    // }
    // if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
    // {
    //     data.statusCode = 501;
    //     return 1;
    // }
    // data.statusCode = "200";
    return 0;
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
    data.contentType = "application/octet-stream";
    if (pos != std::string::npos)
    {
        extension = path.substr(pos + 1);
        if (data.mime.count(extension))
            data.contentType = data.mime[extension];
    }
}

int deleteHandler(Record &log)
{
    struct stat info;

    if (access(log.data.realPath.c_str(), F_OK) == -1)
    {
        log.data.statusCode = 404;
        return 1;
    }
    if (stat(log.data.realPath.c_str(), &info) == -1)
    {
        log.data.statusCode = 500;
        return 1;
    }
    if (S_ISDIR(info.st_mode))
    {
        log.data.statusCode = 403;
        return 1;
    }
    else if (access(log.data.realPath.c_str(), W_OK) == -1)
    {
        log.data.statusCode = 403;
        return 1;
    }
    else if (std::remove(log.data.realPath.c_str()))
    {
        log.data.statusCode = 500;
        return 1;
    }
    log.buffer = "File deleted successfully!\n";
    log.data.bodySize = log.buffer.length();
    log.data.contentType = log.data.mime["txt"];
    log.status = CLOSED_RS;

    return 0;
}

std::string getRandName()
{
    static size_t count;
    std::stringstream ss;
    time_t tStamp = time(NULL);
    struct tm *currentTime = localtime(&tStamp);
    char arr[9];

    strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    ss << count++;
    randName += '_';
    randName += ss.str();
    return randName;
}

int postBody(Record &log)
{
    char buff[RDWR_SIZE];
    int bytes = read(log.fileFd, buff, sizeof(buff) - 1);
    buff[bytes] = '\0';
    if (bytes == -1)
    {
        log.data.statusCode = 500;
        
        // close(log.fd);
        // close(log.fileFd);
        return 1;
    }
    if ((write(log.fd, buff, bytes)) == -1)
    {
        log.data.statusCode = 500;
        // close(log.fd);
        // close(log.fileFd);

        return 1;
    }
    if (bytes < RDWR_SIZE - 1)
    {

        log.status = CLOSED_RS;
        log.method = NONE_RS;
        // close(log.fd);
        // close(log.fileFd);
    }
    else
    {
        log.status = ACTIVE_RS;
        log.method = POST_RS;
    }
    return 0;
}

int postHandler(Record &log, Request &req)
{
    struct stat info;
    if (!log.data.uploadOn)
    {
        log.data.statusCode = 403;
        return 1;
    }
    if (stat(log.data.realPath.c_str(), &info) != -1)
    {
        if (S_ISDIR(info.st_mode))
        {
            log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
            if (log.fileFd == -1)
            {
                log.data.statusCode = 500;
                return 1;
            }
            std::string randName = getRandName();
            std::string tmpStr = log.data.realPath + '/' + randName + ".txt";
            log.fd = open(tmpStr.c_str(), O_WRONLY | O_CREAT, 0644);
            if (log.fd == -1)
            {
                log.data.statusCode = 500;
                return 1;
            }
            if (postBody(log))
                return 1;
            log.data.contentType = log.data.mime["txt"];
        }
        else
        {
            log.data.statusCode = 403;
            return 1;
        }
    }
    else
    {
        log.fd = open(log.data.realPath.c_str(), O_WRONLY | O_CREAT, 0644);
        if (log.fd == -1)
        {
            log.data.statusCode = 403; // -definitely 500---- ask
            return 1;
        }
        log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (log.fileFd == -1)
        {
            log.data.statusCode = 500;
            return 1;
        }
        if (postBody(log))
            return 1;
        getContentType(log.data, log.data.realPath);
    }
    log.data.statusCode = 201;
    return 0;
}

int getFile(Record &log)
{
    char buffer[RDWR_SIZE];
    int bytes = 0;
    bytes = read(log.fd, buffer, sizeof(buffer));
    if (bytes == -1)
    {
        log.data.statusCode = 500;
        // close(log.fd);
        if (log.method == GET_LIST_RS)
            std::remove(log.fileName.c_str());
        return 1;
    }

    log.buffer.append(buffer, bytes);
    if (bytes < RDWR_SIZE)
    {
        log.status = CLOSED_RS;
        // close(log.fd);
        if (log.method == GET_LIST_RS)
            std::remove(log.fileName.c_str());
    }
    else if (log.status != ACTIVE_RS)
    {
        log.status = ACTIVE_RS;
        log.method = GET_RS;
    }
    return 0;
}

std::string fileNameHandler(std::string fileName)
{
    unsigned char c;
    char s[17] = "0123456789ABCDEF";
    for (size_t i = 0; i < fileName.length(); i++)
    {
        if (!isalpha(fileName[i]) && !isdigit(fileName[i]) &&
            fileName[i] != '.' && fileName[i] != '-' && fileName[i] != '_')
        {
            c = fileName[i];
            fileName[i] = '%';
            fileName.insert(fileName.begin() + i + 1, s[c / 16]);
            fileName.insert(fileName.begin() + i + 2, s[c % 16]); // und
        }
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

int postFilesList(Record &log)
{
    struct dirent *rdir;
    std::string tmp;
    std::string buff;
    std::string tmpBuff;
    std::string tmpPath = log.data.location + log.data.uri;
    if (tmpPath[tmpPath.length() - 1] == '/')
        tmpPath.resize(tmpPath.length() - 1);
    tmpPath = cleanPath(tmpPath);
    while ((rdir = readdir(log.dir)))
    {
        tmp = rdir->d_name;
        if (tmp == "." || tmp == "..")
            continue;
        buff = "<a href=\"" + tmpPath + '/' + fileNameHandler(tmp) + "\">" + tmp + "</a><br>\n";
        tmpBuff += buff;
        if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
        {
            // close(log.fileFd);
            // closedir(log.dir);
            log.method = NONE_RS;
            return 1;
        }
        if (tmpBuff.length() > RDWR_SIZE)
        {
            log.status = ACTIVE_RS;
            log.method = POST_LIST_RS;
            return 0;
        }
    }
    buff =
        "</body>\n"
        "</html>\n";
    if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
    {
        // close(log.fileFd);
        // closedir(log.dir);
        log.method = NONE_RS;
        return 1;
    }
    log.method = GET_LIST_RS;
    // close(log.fileFd);
    // closedir(log.dir);

    struct stat info;
    if (stat(log.fileName.c_str(), &info) == -1)
        return 1;
    log.data.bodySize = info.st_size;
    log.fd = open(log.fileName.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        log.method = NONE_RS;
        return 1;
    }
    return 0;
}

int autoIndex(Record &log, playloadData &data)
{
    log.fileName = LIST_FILE_PATH;
    log.fileName += "bodyList_" + getRandName();
    log.fileFd = open(log.fileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log.fileFd == -1)
    {
        std::cerr << "bahmaz\n";
        return 1;
    }
    log.dir = opendir(data.realPath.c_str());
    if (!log.dir)
    {
        std::remove(log.fileName.c_str());
        return 1;
    }
    data.contentType = data.mime["html"];
    std::string buff;
    buff =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>File List</title>\n"
        "</head>\n"
        "<body>\n"
        "<h1>File List</h1>\n"
        "<a href=\"../\">../</a><br>\n";
    if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
    {
        // close(log.fileFd);
        // closedir(log.dir);
        std::remove(log.fileName.c_str());
        return 1;
    }
    if (postFilesList(log))
    {
        std::remove(log.fileName.c_str());
        
        return 1;
    }
    if (log.method == GET_LIST_RS)
    {
        if (getFile(log))
            return 1;
    }
    return 0;
}

int tryIndex(Record &log, playloadData &data)
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
            return 1;
        }
        if (S_ISDIR(info.st_mode))
            continue;
        else if (access(pathIndex.c_str(), R_OK) == -1)
        {
            data.statusCode = 403;
            return 1;
        }
        log.fd = open(pathIndex.c_str(), O_RDONLY);
        if (log.fd == -1)
        {
            data.statusCode = 500;
            return 1;
        }
        data.bodySize = info.st_size;
        getContentType(data, pathIndex);
        break;
    }
    if (log.fd != -1)
    {
        if (getFile(log))
            return 1;
    }
    else
        return 0;
    return 1;
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
    if (stat(data.realPath.c_str(), &info))
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
            data.statusCode = 403;//////////////////
        }
        return 1;
    }
    data.bodySize = info.st_size;
    getContentType(data, data.realPath);
    return 0;
}

int getHandler(Record &log, playloadData &data)
{
    int check;
    check = validateFile(log, data);
    if (data.statusCode != 200)
        return 1;
    if (check)
        return 0;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.statusCode = 500;
        return 1;
    }
    if (getFile(log))
        return 1;
    return 0;
}

void findLongestMatch(playloadData &data, Config &conf, std::string reqPath)
{
    std::map<int, std::string> locs;
    if (reqPath.empty())
        reqPath = "/";
    //location : /server
    //req.path = /server/boha.txt
    for (size_t i = 0; i < conf.location.size(); i++)
    {
        std::string name = conf.location[i].location_name;
        size_t f = reqPath.find(name);
        if (f == 0 && (reqPath.length() == name.length() || reqPath[name.length()] == '/' || name == "/"))
        {
            locs[conf.location[i].location_name.length()] = conf.location[i].location_name;
        }
    }
    if (locs.empty())
    {
        data.uri = reqPath.substr(1);
        if (data.uri[0] != '/')
            data.uri.insert(0, "/");
        return ;
    }
    else
        data.location = locs.rbegin()->second;
    data.uri = reqPath.substr(data.location.length());
    if (data.uri[0] != '/')
        data.uri.insert(0, "/");
    
}

void getImportantData(Config &conf, playloadData &data)
{
    data.loginToken = "/portf/html/login.html";//tmp
    data.redirectTokens.clear();
    data.redirectTokens.push_back("/portf/html/index.html");//tmp
    data.redirectTokens.push_back("/portf/nothing");//tmp
    data.redirectTokens.push_back("/portf/nothing");//tmp
    if (data.location.empty())
    {
        data.location = "/";
        data.methods = conf.methods;
        data.root = conf.root;
        data.index = conf.index;
        data.upload = conf.uploadPath;
        data.autoIndex = conf.autoIndex;
        data.errorPages = conf.error_page;
        data.retPath = conf.retPath;
        data.retCode = conf.retCode;
        data.cgiExt = conf.cgiExt;
        data.cgiPath = conf.cgiPath;
        // data.loginToken = conf.loginToken;
        // data.redirectTokens = conf.redirectTokens;
        return;
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
            // data.loginToken = conf.location[i].conf.loginToken;
            // data.redirectTokens = conf.location[i].conf.redirectTokens;
            break;
        }
    }
    // std::cout << data.loginToken << "\n";
    // for(size_t i = 0; i < data.redirectTokens.size(); i++)
    // {
    //     std::cout << data.redirectTokens[i] << "\n";
    // }
}

int checkErrorFile(playloadData &data, Config &conf)
{
    (void)conf;// ***************remove***************
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
        data.bodyStr.append(buff, bytes);
    close(fd);
    if (bytes < 0)
        return 1;
    data.bodySize = info.st_size;
    return 0;
}

void chooseErrorHandler(Config &conf, playloadData &data)
{
    if (data.errorPages.empty())
        defaultErrorPage(data);
    else if (checkErrorFile(data, conf))
        defaultErrorPage(data);
}

int checkCgi(playloadData &data, std::string realNames, std::string &reqPath)
{
    for (size_t i = 0; i < data.cgiExt.size(); i++)
    {
        size_t k = data.realPath.rfind(data.cgiExt[i]);
        if (k != std::string::npos 
            && (k + data.cgiExt[i].size() == data.realPath.size()
            || data.realPath[k + data.cgiExt[i].size()] == '?'))
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

int resolvePath(playloadData &data, Config &conf, std::string &reqPath)
{
    (void)conf;
    struct stat info;
    std::stringstream ssPath(data.root + data.uri);
    std::vector<std::string> realNames;
    std::string tmp;
    size_t index = 0;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "")
            continue;
        if (tmp == ".." || tmp == ".")
        {
            data.statusCode = 403;
            return 1;
        }
        else
            realNames.push_back(tmp);
        index++;
    }
    for (size_t i = 0; i < realNames.size(); i++)
    {
        data.realPath += '/' + realNames[i];
        if (!checkCgi(data, realNames[i], reqPath))
            continue;
        if (stat(data.cgiScriptFile.c_str(), &info) != -1)
        {
            if (S_ISDIR(info.st_mode))
                continue;
            // data.isCgi = true;
            // return 0;
        }
        // else
        // {
        //     if (!data.cgiInfo.empty())// check ask oussama
        //     {
        //         data.statusCode = "404";
        //         return 1;
        //     }
        // }
        data.isCgi = true;
        return 0;
    }
    if (!index)
        data.realPath = "/";
    data.uploadOn = 0;
    if (data.upload.empty())
        return 0;
    std::string rootUploadDir = cleanPath(data.root + data.upload) + '/';
    std::string compare = data.realPath.substr(0, rootUploadDir.length());
    if (compare[compare.length() - 1] != '/')
        compare.push_back('/');
    // std::cout << "rootUploadDir: " << rootUploadDir << "  data.realPath: " << data.realPath << " compare: " << compare << std::endl;
    // if (rootUploadDir[rootUploadDir.length() - 1] != '/')
    //     rootUploadDir.push_back('/'); //Fix
    if (compare == rootUploadDir)
        data.uploadOn = 1;
    return 0;
}


int authenCheck(Record &log, Request &req, SessionsList &sList)
{
    // bool logged = true;
    (void) sList;
    if (log.data.loginToken == req.path && req.isLoggedCookie)
    {
        if (req.method != "GET")
            return 0;
        log.data.retPath = log.data.redirectTokens[0];
        log.data.retValid = true;
        log.data.retCode = 302;
        return 1;
    }
    else
    {
        if (req.method == "DELETE")
            return 0;
        // std::cout << req.isLoggedCookie << "\n";
        for (size_t i = 0; i < log.data.redirectTokens.size(); i++)
        {
                
            if (log.data.redirectTokens[i] == req.path && req.isLoggedCookie == false)
            {
                log.data.retPath = log.data.loginToken;
                log.data.retValid = true;
                log.data.retCode = 302;
                return 1;
            }
            else if (log.data.redirectTokens[i] == req.path && req.isLoggedCookie)
            {
                if (req.method == "POST")
                    req.method = "GET";
                return 0;
            }
        }
    }
    return 0;
}

int responseHandler(Webserv &Web, Record &log, Request &req, Config &conf, size_t &i, SessionsList &sList)
{
    log.data.retValid = false;
    log.data.isCgi = false;
    log.data.statusCode = req.status;
    if (tolowerStr(req.headers["connection"]) == "keep-alive")
        log.data.connection = "keep-alive";
    findLongestMatch(log.data, conf, cleanPath(req.path));//should be removed
    getImportantData(conf, log.data);
    if (log.data.statusCode != 200)
        return 1;
    if (!log.data.methods[req.method])
    {
        log.data.statusCode = 405;
        return 1;
    }
    if (!log.data.retPath.empty())
    {
        if (req.method == "POST")
        {
            if (access(req.bodyPath.c_str(), F_OK) != -1)
                std::remove(req.bodyPath.c_str());
        }
        log.data.retValid = true;
        return 0;
    }
    if (authenCheck(log, req, sList))
        return 0;
    if (resolvePath(log.data, conf, req.path))
        return 1;
    if (log.data.isCgi)
    {
        if (cgiHandler(Web, log, req, conf.listen_Port,i))
        {
            log.cgiData.cgiValid = false;
            return 1;
        }
        return 0;
    }
    if (req.method == "GET")
    {
        if (getHandler(log, log.data))
            return 1;
    }
    else if (req.method == "POST")
    {
        if (postHandler(log, req))
        {
            if (access(req.bodyPath.c_str(), F_OK) != -1)
                std::remove(req.bodyPath.c_str());
            log.method = NONE_RS;
            return 1;
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
            return 1;
    }
    return 0;
}

void generateCgiResponse(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Request &req)
{
    std::stringstream res;
    (void)data;
    res << "HTTP/1.0 " << data.statusCode << ' ' << data.table[data.statusCode] + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    std::map<std::string, std::string>::iterator iter;
    int ok = 0;
    for (iter = cgiData.cgHeaders.begin(); iter != cgiData.cgHeaders.end(); iter++)
    {
        res << iter->first << ": " << iter->second << "\r\n";
        if (tolowerStr(iter->first) == "content-length")
            ok = 1;
    }
    if (!ok)
    {
        res << "Content-Length: " << data.bodySize << "\r\n";
    }
    if (req.headers.count("cookie"))
        res << "Set-Cookie: " << req.headers["cookie"] << "\r\n"; // set cookie
    res << "\r\n";
    ret_resp = res.str();
}

void generateRetResponse(playloadData &data, std::string &ret_resp, Request &req)
{
    std::stringstream res;
    if (data.retCode != 301 && data.retCode != 302)
        res << "HTTP/1.0 501 Not Implemented" << "\r\n";
    else
    {
        res << "HTTP/1.0 " << data.retCode << " " << data.table[data.retCode] << "\r\n";
        res << "Location: " << data.retPath << "\r\n";
    }
    if (req.headers.count("cookie"))
        res << "Set-Cookie: " << req.headers["cookie"] << "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Length: 0\r\n";
    res << "\r\n";
    ret_resp = res.str();
    return;
}

void generateResponse(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Record &log, Request &req)
{
    if (data.statusCode != 200 && data.statusCode != 201)
        data.connection = "close";
    if (data.connection == "keep-alive")
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
        data.retValid = false;
        return;
    }
    std::stringstream res;
    res << "HTTP/1.0 " << data.statusCode << ' ' << data.table[data.statusCode] + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    if (req.headers.count("cookie"))
        res << "Set-Cookie: " << req.headers["cookie"] << "\r\n";
    res << "\r\n";
    if ((log.method != POST_RS && log.method != POST_LIST_RS) || log.status == ERROR_RS)
    {
        ret_resp = res.str();
    }
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
            chooseErrorHandler(conf, log.data);
            log.data.contentType = log.data.mime["html"];
            log.status = ERROR_RS;
        }
    }
    else if (log.status == ACTIVE_RS || log.status == CGI_ACTIVE)
    {
        if (log.method == CGI_RDWR)
        {
            if (cgiProcess(Web, log, req,indexClnt))
            {
                log.data.contentType = log.data.mime["html"];
                chooseErrorHandler(conf, log.data);
                log.status = ERROR_RS;
            }
        }
        if (log.method == POST_LIST_RS)
        {
            if (postFilesList(log))
            {
                std::remove(log.fileName.c_str());
                log.data.statusCode = 500;
                chooseErrorHandler(conf, log.data);
                log.status = ERROR_RS;
            }
        }
        if (log.method == GET_RS || log.method == GET_LIST_RS)
        {
            if (getFile(log))
            {
                // may be something
                log.status = ERROR_RS;
            }
        }
        else if (log.method == POST_RS)
        {
            if (postBody(log))
            {
                // may be something
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
    if (log.method == POST_LIST_RS || log.status == CGI_ACTIVE)
        return "";

    if (pastStatus == NEW_RS
        // || (log.method == NONE_RS && log.status == ERROR_RS) /!\  TEST
        || (pastMethod == POST_RS && log.method == NONE_RS)
        || (pastMethod == POST_LIST_RS && (log.method == GET_LIST_RS || log.method == NONE_RS))
        || (pastStatus == CGI_ACTIVE && log.status != CGI_ACTIVE))
    {
        generateResponse(log.cgiData, log.data, ret_resp, log, req);
        if (log.status == ERROR_RS)
            ret_resp += log.data.bodyStr;
        else
            ret_resp += log.buffer;
    }
    else if (log.status == ERROR_RS)
        ret_resp.clear();
    else
        ret_resp = log.buffer;
    log.buffer.clear();
    return (ret_resp);
}
