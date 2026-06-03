#include "../server/server.hpp"

void Request::clean() // add youssef
{
    if (access(bodyFile.c_str(), F_OK) != -1)
        std::remove(bodyPath.c_str());
}

void defaultErrorPage(playloadData &data)
{
    data.bodyStr =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>Error page</title>\n"
        "</head>\n"
        "<body>\n";
    data.bodyStr += "<h1 style=\"text-align: center;\">" + data.statusCode + ' ' + data.statusName + "</h1>\n";
    data.bodyStr +=
        "<hr>"
        "</body>\n"
        "</html>\n";
    data.bodySize = data.bodyStr.length();
}

int readyChecks(Request &req, playloadData &data)
{
    if (req.status == 400)
    {
        data.statusCode = "400";
        data.statusName = "Bad Request"; // on map
        return 1;
    }
    else if (req.status == 500)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    else if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
    {
        data.statusCode = "501";
        data.statusName = "Not Implemented";
        return 1;
    }
    else if (!data.methods[req.method])
    {
        data.statusCode = "405";
        data.statusName = "Method Not Allowed";
        return 1;
    }
    data.statusCode = "200";
    data.statusName = "OK";
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

int deleteHandler(Record &log, playloadData &data)
{
    struct stat info;

    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.statusCode = "404";
        data.statusName = "Not Found";
        return 1;
    }
    if (stat(data.realPath.c_str(), &info) == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    if (S_ISDIR(info.st_mode))
    {
        data.statusCode = "403";
        data.statusName = "Forbidden";
        return 1;
    }
    else if (access(data.realPath.c_str(), W_OK) == -1)
    {
        data.statusCode = "403";
        data.statusName = "Forbidden";
        return 1;
    }
    else if (std::remove(data.realPath.c_str()))
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    log.buffer = "File deleted successfully!\n";
    data.bodySize = log.buffer.length();
    data.contentType = data.mime["txt"];
    log.status = CLOSED_RS;
    data.statusCode = "200";
    data.statusName = "Ok";
    return 0;
}

std::string getRandomName(Config &conf)
{
    std::stringstream ss;
    time_t tStamp = time(NULL);
    struct tm *currentTime = localtime(&tStamp);
    char arr[9];

    strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    ss << conf.count++;
    randName += '_';
    randName += ss.str();
    // stat
    return randName;
}

int postBody(Record &log, playloadData &data)
{
    const int size = 8192;
    char buff[size];
    int bytes = read(log.fileFd, buff, sizeof(buff));
    if (bytes == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        // close(log.fd);
        // close(log.fileFd);
        return 1;
    }
    // std::cout << buff << std::endl;
    if ((write(log.fd, buff, bytes)) == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        // close(log.fd);
        // close(log.fileFd);
        return 1;
    }
    if (bytes < size)
    {
        log.status = CLOSED_RS;
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

int postHandler(Record &log, playloadData &data, Request &req, Config &conf)
{
    struct stat info;
    // std::cout << req.headers["content-length"] << std::endl;
    if (!data.uploadOn)
    {
        data.statusCode = "403";
        data.statusName = "Forbidden";
        return 1;
    }
    if (stat(data.realPath.c_str(), &info) != -1)
    {
        if (S_ISDIR(info.st_mode))
        {
            log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
            if (log.fileFd == -1)
            {
                data.statusCode = "500";
                data.statusName = "Internal Server Error";
                return 1;
            }
            std::string randName = getRandomName(conf);
            std::string tmpStr = data.realPath + '/' + randName + ".txt";
            log.fd = open(tmpStr.c_str(), O_WRONLY | O_CREAT, 0644);
            if (log.fd == -1)
            {
                data.statusCode = "500";
                data.statusName = "Internal Server Error";
                return 1;
            }
            if (postBody(log, data))
                return 1;
            data.contentType = data.mime["txt"];
        }
        else
        {
            // std::cout << "yoyoy\n\n\n";
            data.statusCode = "403";
            data.statusName = "Forbidden";
            return 1;
        }
    }
    else
    {
        log.fd = open(data.realPath.c_str(), O_WRONLY | O_CREAT, 0644);
        if (log.fd == -1)
        {
            data.statusCode = "403";
            data.statusName = "Forbidden";
            return 1;
        }
        log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (log.fileFd == -1)
        {
            // std::cout << req.bodyPath<<std::endl;
            data.statusCode = "500";
            data.statusName = "Internal Server Error";
            return 1;
        }
        if (postBody(log, data))
            return 1;
        getContentType(data, data.realPath);
    }
    data.statusCode = "201";
    data.statusName = "Created";
    return 0;
}

int getFile(Record &log, playloadData &data)
{
    const int size = 8192;
    char buffer[size];
    int bytes = 0;
    bytes = read(log.fd, buffer, size);
    if (bytes == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        // close(log.fd);
        if (log.method == GET_LIST_RS)
            std::remove(log.fileName.c_str());
        return 1;
    }
    log.buffer.append(buffer, bytes);
    if (bytes < size)
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
        if (tmp == "" || tmp == "." || tmp == "..") // you can remove it
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

int postFilesList(Record &log, playloadData &data)
{
    struct dirent *rdir;
    std::string tmp;
    std::string buff;
    std::string tmpBuff;
    std::string tmpPath = data.location + data.uri;
    if (tmpPath[tmpPath.length() - 1] == '/')
        tmpPath.resize(tmpPath.length() - 1);
    tmpPath = cleanPath(tmpPath);
    while ((rdir = readdir(log.dir)))
    {
        tmp = rdir->d_name;
        if (tmp == "." || tmp == "..")
            continue ;
        buff = "<a href=\"" + tmpPath + '/' + fileNameHandler(tmp) + "\">" + tmp + "</a><br>\n";
        tmpBuff += buff;
        if ((write(log.fileFd, buff.c_str(), buff.length())) == -1)
        {
            // close(log.fileFd);
            // closedir(log.dir);
            log.method = NONE_RS;
            return 1;
        }
        if (tmpBuff.length() > 8192)
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
    data.bodySize = info.st_size;
    log.fd = open(log.fileName.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        log.method = NONE_RS;   
        return 1;
    }
    return 0;
}

int autoIndex(Record &log, playloadData &data, Config &conf)
{
    log.fileName = LIST_FILE_PATH;
    log.fileName += "bodyList_" + getRandomName(conf);
    log.fileFd = open(log.fileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log.fileFd == -1)
        return 1;
    log.dir = opendir(data.realPath.c_str());
    if (!log.dir)
    {
        // close(log.fileFd);
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
    if (postFilesList(log, data))
    {
        std::remove(log.fileName.c_str());
        return 1;
    }
    if (log.method == GET_LIST_RS)
    {
        if (getFile(log, data))
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
            data.statusCode = "500";
            data.statusName = "Internal Server Error";
            return 1;
        }
        if (S_ISDIR(info.st_mode))
            continue;
        else if (access(pathIndex.c_str(), R_OK) == -1)
        {
            data.statusCode = "403";
            data.statusName = "Forbidden";
            return 1;
        }
        log.fd = open(pathIndex.c_str(), O_RDONLY);
        if (log.fd == -1)
        {
            data.statusCode = "500";
            data.statusName = "Internal Server Error";
            return 1;
        }
        data.bodySize = info.st_size;
        getContentType(data, pathIndex);
        break;
    }
    if (log.fd != -1)
    {
        if (getFile(log, data))
            return 1;
    }
    else
        return 0;
    return 1;
}

int validateFile(Record &log, playloadData &data, Config &conf)
{
    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.statusCode = "404";
        data.statusName = "Not Found";
        return 1;
    }
    else if (access(data.realPath.c_str(), R_OK) == -1)
    {
        data.statusCode = "403";
        data.statusName = "Forbidden";
        return 1;
    }
    struct stat info;
    if (stat(data.realPath.c_str(), &info))
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    if (S_ISDIR(info.st_mode))
    {
        if (tryIndex(log, data))
            return 1;
        if (data.autoIndex)
        {
            if (autoIndex(log, data, conf))
            {
                data.statusCode = "500";
                data.statusName = "Internal Server Error";
                return 1;
            }
        }
        else
        {
            data.statusCode = "404";
            data.statusName = "Not Found";
        }
        return 1;
    }
    data.bodySize = info.st_size;
    getContentType(data, data.realPath);
    return 0;
}

int getHandler(Record &log, playloadData &data, Config &conf)
{
    int check;
    check = validateFile(log, data, conf);
    if (data.statusCode != "200")
        return 1;
    if (check)
        return 0;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    if (getFile(log, data))
        return 1;
    return 0;
}

void findLongestMatch(playloadData &data, Config &conf, std::string reqPath) // tests
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
// std::cout << data.uri << std::endl;
// std::cout << reqPath.length() << '\n' << data.location << '\n';
// std::cout << data.uri << std::endl;
// exit(0);
    
void getImportantData(Config &conf, playloadData &data)
{
    data.cgiExt.push_back(".py");      // tmp
    data.cgiExt.push_back(".cgi");     // tmp
    data.cgiPath.push_back("/usr/bin/python3"); // tmp
    data.cgiPath.push_back("/bin/lb"); // tmp
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
        // data.cgiExt = conf.cgiExt;
        // data.cgiPath = conf.cgiPath;
        return;
    }
    for (size_t i = 0; i < conf.location.size(); i++)
    {

        if (conf.location[i].location_name == data.location)
        {
            // if (conf.location[i].conf.error_page.empty())
                
            data.errorPages = conf.location[i].conf.error_page;
            data.methods = conf.location[i].conf.methods;
            data.index = conf.location[i].conf.index;
            data.root = conf.location[i].conf.root;
            data.upload = conf.location[i].conf.uploadPath;
            data.autoIndex = conf.location[i].conf.autoIndex;
            data.retPath = conf.location[i].conf.retPath;
            data.retCode = conf.location[i].conf.retCode;
            // data.cgiExt = conf.location[i].conf.cgiExt;
            // data.cgiPath = conf.location[i].conf.cgiPath;
            break;
        }
    }
}

int checkErrorFile(playloadData &data, Config &conf)
{
    int code = atoi(data.statusCode.c_str());
    std::string file = data.root;
    if (file[file.length() - 1] != '/')
        file += '/';
    std::string page = data.errorPages[code];
    if (page.empty())
    {
        if (conf.error_page[code].empty())
            return 1;
        else
            page = conf.error_page[code];
    }
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
    char buff[8192]; // use more
    int bytes = 0;
    while ((bytes = read(fd, buff, sizeof(buff) - 1)) > 0)
        data.bodyStr.append(buff, bytes);
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
        if (k != std::string::npos && (k + data.cgiExt[i].size() == data.realPath.size() || data.realPath[k + data.cgiExt[i].size()] == '?'))
        {
            data.cgiInfo = data.realPath.substr(k + data.cgiExt[i].size());
            data.cgiScriptFile = data.realPath.substr(0, k + data.cgiExt[i].size());
            data.cgiInfo += reqPath.substr(reqPath.find(realNames) + realNames.length());
            data.cgiFile = cleanPath(reqPath.substr(0, reqPath.length() - data.cgiInfo.length()));
            data.cgiExecutePath = data.cgiPath[i];
            // std::cout << data.cgiFile << std::endl;
            // std::cout << data.cgiScriptFile << std::endl;
            return 1;
        }
    }
    return 0;
}

int resolvePath(playloadData &data, Config &conf, std::string &reqPath) // path
{
    (void)conf;
    std::string tmpPath;
    struct stat info;
    
    // stat(data.realPath.c_str(), &info);// cgi
    std::stringstream ssPath(data.root + data.uri);
    std::vector<std::string> realNames;
    std::string tmp;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "" || tmp == ".")
            continue;
        if (tmp == "..")
        {
            data.statusCode = "403";
            data.statusName = "Forbidden";
            return 1;
        }
        else
            realNames.push_back(tmp);
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
            data.isCgi = true;
            return 0;
        }
        else
        {
            if (!data.cgiInfo.empty())
            {
                data.statusCode = "404";
                data.statusName = "Not Found";
                return 1;
            }
            data.isCgi = true;
            return 0;
        }
    }
    data.uploadOn = 0;
    if (data.upload.empty())
        return 0;
    if (data.root[data.root.length() - 1] != '/' && data.upload[0] != '/')
        data.upload.insert(0, "/");
    std::string rootUploadDir = data.root + data.upload;
    std::string compare = data.realPath.substr(0, rootUploadDir.length());
    // std::cout << rootUploadDir << std::endl;
    // std::cout << compare << std::endl;
    if (compare == rootUploadDir)
        data.uploadOn = 1;
    return 0;
}

int responseHandler(CgiHandler &cgiData, Server &obj, Record &log, Request &req, Config &conf, playloadData &data)
{
    if (req.headers["connection"] == "Keep-Alive")
        data.connection = "Keep-Alive";
    findLongestMatch(data, conf, cleanPath(req.path));
    getImportantData(conf, data);
    data.retValid = 0;
    data.isCgi = false;
    if (!data.retPath.empty())
    {
        if (data.retCode != 301 && data.retCode != 302)
        {
            data.statusCode = "501";
            data.statusName = "Not Implemented";
            return 1;
        }
        data.retValid = true;
        return 0;
    }
    if (readyChecks(req, data))
        return 1;
    if (resolvePath(data, conf, req.path))
        return 1;
    if (data.isCgi)
    {
        if (cgiHandler(log, cgiData, obj, data, req, conf.listen_Port))
        {
            cgiData.cgiValid = false;
            return 1;
        }
        return 0;
    }
    if (req.method == "GET")
    {
        if (getHandler(log, data, conf))
            return 1;
    }
    else if (req.method == "POST")
    {
        if (postHandler(log, data, req, conf))
            return 1;
    }
    else if (req.method == "DELETE")
    {
        if (deleteHandler(log,data))
            return 1;
    }
    return 0;
}

void generateCgiResponse(CgiHandler &cgiData, playloadData &data, std::string &ret_resp)
{
    std::stringstream res;
    (void)data;
    res << "HTTP/1.0 " << cgiData.cgStatusCode << ' ' << cgiData.cgStatusName + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    std::map<std::string, std::string>::iterator iter;
    for(iter = cgiData.cgHeaders.begin(); iter != cgiData.cgHeaders.end(); iter++)
        res << iter->first << ": " << iter->second << "\r\n";
    res << "\r\n";
    ret_resp = res.str();
}

void generateRetResponse(playloadData &data, std::string &ret_resp)
{
    std::stringstream res;
    if (data.retCode != 301 && data.retCode != 302)
    {
        res << "HTTP/1.0 501 Not Implemented" << "\r\n";
        res << "Connection: " << data.connection << "\r\n";
        res << "Content-Length: 0\r\n";
        res << "\r\n";
    }
    else
    {
        res << "HTTP/1.0 " << data.retCode;
        if (data.retCode == 301)
            res << " Moved Permanently\r\n";
        else if (data.retCode == 302)
            res << " Moved Temporarily\r\n";
        res << "Connection: " << data.connection << "\r\n";
        res << "Content-Length: 0\r\n";
        res << "Location: " << data.retPath << "\r\n";
        res << "\r\n";
    }
    ret_resp = res.str();
    return ;
}

void generateResponse(CgiHandler &cgiData, playloadData &data, std::string &ret_resp, Record &log)
{
    // if (log.method == POST_LIST_RS)
    //     data = data;
    // if ((pastMethod == POST_LIST_RS && log.method == GET_LIST_RS)
    //     || (log.method == NONE_RS && log.status == ERROR_RS))
    //     data.bodySize+=0;
    if (cgiData.cgiValid)
    {
        generateCgiResponse(cgiData, data, ret_resp);
        return ;
    }

    if (data.retValid)
    {
        generateRetResponse(data, ret_resp);
        data.retValid = 0;
        return ;
    }
    std::stringstream res;
    res << "HTTP/1.0 " << data.statusCode << ' ' << data.statusName + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    res << "\r\n";
    if (data.connection == "Keep-Alive")
        log.clientState = ALIVE;
    if (log.method != POST_LIST_RS || log.status == ERROR_RS)
        ret_resp = res.str();
}

std::string respond(Server &obj, playloadData &data, Record &log, Config &conf, Request &req)
{

    // std::cout << req.path << std::endl;
    std::string ret_resp;
    t_state pastStatus = log.status;
    t_state pastMethod = log.method;

    if (log.status == NEW_RS)
    {

        data.bodySize = 0;
        loadExtensions(data);
        data.connection = "Close";
        if (responseHandler(log.cgiData, obj, log, req, conf, data))
        {
            chooseErrorHandler(conf, data);
            data.contentType = data.mime["html"];
            log.status = ERROR_RS;
        }
        // std::cout << "lpoooooooooooooooooooo\n\n";
    }
    else if (log.status == ACTIVE_RS || log.status == CGI_ACTIVE)
    {
        // std::cout << "youssef \n\n";
        if (log.method == CGI_RDWR)
        {
            if (cgiProcess(log.cgiData, log, obj, data, req))
            {
                data.contentType = data.mime["html"];
                chooseErrorHandler(conf, data);
                log.status = ERROR_RS;
            }
        }
        if (log.method == POST_LIST_RS)
        {
            if (postFilesList(log, data))
            {
                std::remove(log.fileName.c_str());
                data.statusCode = "500";
                data.statusName = "Internal Server Error";
                chooseErrorHandler(conf, data);
                log.status = ERROR_RS;
            }
        }
        if (log.method == GET_RS || log.method == GET_LIST_RS)
        {
            if (getFile(log, data))
            {
                // may be something
                log.status = ERROR_RS;
            }
        }
        else if (log.method == POST_RS)
        {
            if (postBody(log, data))
            {
                // may be something
                log.status = ERROR_RS;
            }
            // remove bodyfile
        }
    }

    if (log.method == POST_LIST_RS || log.status == CGI_ACTIVE)
        return "";

    if (pastStatus == NEW_RS
        // || (log.method == NONE_RS && log.status == ERROR_RS) /!\  TEST
        || (pastMethod == POST_LIST_RS && (log.method == GET_LIST_RS || log.method == NONE_RS))
        || (pastStatus == CGI_ACTIVE && log.status != CGI_ACTIVE))
    {
        generateResponse(log.cgiData, data, ret_resp, log);
        if (log.status == ERROR_RS)
            ret_resp += data.bodyStr;
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
