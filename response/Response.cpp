#include "../server/server.hpp"

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
    data.bodyStr += "<h1 style=\"text-align: center;\">" + data.status_code + ' '
                 + data.resMessage + "</h1>\n";
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
        data.status_code = "400";
        data.resMessage = "Bad Request";
        return 1;
    }
    else if (req.status == 500)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return 1;
    }
    else if ((req.method != "GET" && req.method != "POST" && req.method != "DELETE")
        || !data.locationMethods[req.method])
    {
        data.status_code = "405";
        data.resMessage = "Method Not Allowed";
        return 1;
    }
    data.status_code = "200";
    data.resMessage = "Ok";
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
    size_t pos = path.find_last_of('.');
    data.contentType = "application/octet-stream";
    if (pos != std::string::npos)
    {
        extension = path.substr(pos + 1);
        if (data.mime.count(extension))
            data.contentType = data.mime[extension];
    }
}

std::string cleanPath(std::string fullPath)
{
    std::stringstream ssPath(fullPath);
    std::vector<std::string> realNames;
    std::string tmp;
    std::string path;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "" || tmp == "." || tmp == "..")
            continue;
        // if (tmp == "..")// dkhelha m3a continue o hni rasek
        // {
        //     if (!realNames.empty())
        //         realNames.pop_back();
        // }
        else
            realNames.push_back(tmp);
    }

    for (size_t i = 0; i < realNames.size(); i++)
    {
        path += '/' + realNames[i];
    }
    return path;
}

int handleDelete(Record &log, playloadData &data)
{
    struct stat info;

    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.status_code = "404";
        data.resMessage = "Not Found";
        return 1;
    }
    if (stat(data.realPath.c_str(), &info) == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return 1;
    }
    if (S_ISDIR(info.st_mode))
    {
        data.status_code = "403";
        data.resMessage = "Forbidden";
        return 1;
    }
    else if (access(data.realPath.c_str(), W_OK) == -1)
    {
        data.status_code = "403";
        data.resMessage = "Forbidden";
        return 1;
    }
    else if (std::remove(data.realPath.c_str()))
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return 1;
    }
    log.buffer = "File deleted successfully!\n";
    data.bodySize = log.buffer.length();
    data.contentType = data.mime["txt"];
    log.status = CLOSED_RS;
    data.status_code = "200";
    data.resMessage = "Ok";
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
    //stat
    return randName;
}

int postBody(Record &log, playloadData &data)
{
    const int size = 8192;
    char buff[size];
    int bytes = read(log.fileFd, buff, sizeof(buff));
    if (bytes == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        // close(log.fd);
        // close(log.fileFd);
        return 1;
    }
    if ((write(log.fd, buff, bytes)) == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
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

int handlePost(Record &log, playloadData &data, Request &req, Config &conf)
{
    struct stat info;
    if (!data.uploadOn)
    {
        data.status_code = "403";
        data.resMessage = "Forbidden";
        return 1;
    }
    if (stat(data.realPath.c_str(), &info) != - 1)
    {
        if (S_ISDIR(info.st_mode))
        {
            log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
            if (log.fileFd == -1)
            {
                data.status_code = "500";
                data.resMessage = "Internal Server Error";
                return 1;
            }
            std::string randName = getRandomName(conf);
            std::string tmpStr = data.realPath + '/' + randName + ".txt";
            log.fd = open(tmpStr.c_str(), O_WRONLY | O_CREAT, 0644);
            if (log.fd == -1)
            {
                data.status_code = "500";
                data.resMessage = "Internal Server Error";
                return 1;
            }
            if (postBody(log, data))
                return 1;
            data.contentType = data.mime["txt"];
        }
        else
        {
            data.status_code = "403";
            data.resMessage = "Forbidden";
            return 1;
        }
    }
    else
    {
        log.fd = open(data.realPath.c_str(), O_WRONLY | O_CREAT, 0644);
        if (log.fd == -1)
        {
            data.status_code = "403";
            data.resMessage = "Forbidden";
            return 1;
        }
        log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (log.fileFd == -1)
        {
            std::cout << req.bodyPath<<std::endl;
            data.status_code = "500";
            data.resMessage = "Internal Server Error";
            return 1;
        }
        if (postBody(log, data))
            return 1;
        getContentType(data, data.realPath);
    }
    data.status_code = "201";
    data.resMessage = "Created";
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
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
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
        buff = "<a href=\"" + tmpPath + '/' + tmp + "\">" + tmp + "</a><br>\n";
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
    if(stat(log.fileName.c_str(), &info) == -1)
        return 1;
    data.bodySize = info.st_size;
    log.fd = open(log.fileName.c_str(), O_RDONLY);
    if (log.fd == -1)
        return 1;
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
    for(size_t i = 0; i < data.index.size(); i++)
    {
        std::string pathIndex = data.realPath + '/' + data.index[i];
        if (access(pathIndex.c_str(), F_OK) == -1)
            continue ;
        struct stat info;
        if (stat(pathIndex.c_str(), &info) == -1)
        {
            data.status_code = "500";
            data.resMessage = "Internal Server Error";
            return 1;
        }    
        if (S_ISDIR(info.st_mode))
            continue;
        else if (access(pathIndex.c_str(), R_OK) == -1)
        {
            data.status_code = "403";
            data.resMessage = "Forbidden";
            return 1;
        }
        log.fd = open(pathIndex.c_str(), O_RDONLY);
        if (log.fd == -1)
        {
            data.status_code = "500";
            data.resMessage = "Internal Server Error";
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
        data.status_code = "404";
        data.resMessage = "Not Found";
        return 1;
    }
    else if (access(data.realPath.c_str(), R_OK) == -1)
    {
        data.status_code = "403";
        data.resMessage = "Forbidden";
        return 1;
    }
    struct stat info;
    if (stat(data.realPath.c_str(), &info))
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
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
                data.status_code = "500";
                data.resMessage = "Internal Server Error";
                return 1;
            }
        }
        else
        {
            data.status_code = "404";
            data.resMessage = "Not Found";
        }
        return 1;
    }
    data.bodySize = info.st_size;
    getContentType(data, data.realPath);
    return 0;
}

int handleGet(Record &log, playloadData &data, Config &conf)
{
    int check;
    check = validateFile(log, data, conf);
    if (data.status_code != "200")
        return 1;
    if (check)
        return 0;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return 1;
    }
    if (getFile(log, data))
        return 1;
    return 0;
}

void findLongestMatch(playloadData &data, Config &conf, Request &req) //tests
{
    std::map<int, std::string> locs;
    for(size_t i = 0; i < conf.location.size(); i++)
    {
        std::string name = conf.location[i].location_name;
        size_t f = req.path.find(name); 
        if (f == 0 && (req.path.length() == name.length() || req.path[name.length()] == '/'
            || name == "/"))
            locs[conf.location[i].location_name.length()] = conf.location[i].location_name;
    }
    if (locs.empty())
    {
        data.uri = req.path.substr(1);
        if (data.uri[0] != '/')
            data.uri.insert(0, "/");
        return ;
    }
    else
        data.location = locs.rbegin()->second;
    data.uri = req.path.substr(data.location.length());
    if (data.uri[0] != '/')
        data.uri.insert(0, "/");
    // std::cout << data.uri << std::endl;
    // std::cout << req.path.length() << '\n' << data.location << '\n';
    // std::cout << data.uri << std::endl;
    // exit(0);
}

void getImportantData(Config &conf, playloadData &data)
{
    if (data.location.empty())
    {
        data.location = "/";
        data.locationMethods = conf.methods;
        data.locationRoot = conf.root;
        data.index = conf.index;
        data.locationUpload = conf.uploadPath;
        data.autoIndex = conf.autoIndex;
        return ;
    }
    for(size_t i = 0; i < conf.location.size(); i++)
    {
        if (conf.location[i].location_name == data.location)
        {
            if (!conf.location[i].conf.error_page.empty())
                data.errorPages = conf.location[i].conf.error_page;
            else
                data.errorPages = conf.error_page;////
            data.locationMethods = conf.location[i].conf.methods;
            data.locationIndex = i;
            data.index = conf.location[i].conf.index;
            data.locationRoot = conf.location[i].conf.root;
            data.locationUpload = conf.location[i].conf.uploadPath;
            data.autoIndex = conf.location[i].conf.autoIndex;
            break ;
        }
    }
    if (data.locationMethods.empty())///
    {
        data.locationMethods = conf.methods;
    }
}
 
int checkErrorFile(playloadData &data, Config &conf)
{
    int code = atoi(data.status_code.c_str());
    std::string file = data.locationRoot;
    if (file[file.length() - 1] != '/')
        file += '/' ;
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
    char buff[8192];//use more
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

void resolvePath(playloadData &data, Config &conf) // path
{
    (void)conf;
    std::string tmp;
    // std::string tmpPath;
    // struct stat info;
    // stat(data.realPath.c_str(), &info);// cgi
    data.realPath = cleanPath(data.locationRoot + data.uri);
    if (data.locationUpload.empty())
        return ;
    if (data.locationRoot[data.locationRoot.length() - 1] != '/'
        && data.locationUpload[0] != '/')
        data.locationUpload.insert(0, "/");
    std::string rootUploadDir = data.locationRoot + data.locationUpload;
    std::string compare = data.realPath.substr(0, rootUploadDir.length());
    // std::cout << rootUploadDir << std::endl;
    // std::cout << compare << std::endl;
    if (compare == rootUploadDir)
        data.uploadOn = 1;
}

int handleResponse(Record &log, Request &req, Config &conf, playloadData &data)
{
    findLongestMatch(data, conf, req);
    getImportantData(conf, data);
    if (readyChecks(req, data))
        return 1;
    resolvePath(data, conf);
    if (req.headers["connection"] == "Keep-Alive")
        data.connection = "Keep-Alive";
    if (req.method == "GET") 
    {
        if (handleGet(log, data, conf))
            return 1;
    }
    else if (req.method == "POST") 
    {
        if (handlePost(log, data, req, conf))
            return 1;
    }
    else if (req.method == "DELETE")
    {
        if (handleDelete(log, data))
            return 1;
    }
    return 0;
}

void generateResponse(playloadData &data, std::string &ret_resp, Record &log, t_state pastMethod)
{
    // if (log.method == POST_LIST_RS)
    //     data = data;
    (void)pastMethod;
    std::stringstream res;
    // if ((pastMethod == POST_LIST_RS && log.method == GET_LIST_RS)
    //     || (log.method == NONE_RS && log.status == ERROR_RS))
    //     data.bodySize+=0; 
    res << "HTTP/1.0" << ' ' << data.status_code << ' ' << data.resMessage + "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    res << "\r\n";
    if (log.method != POST_LIST_RS || log.status == ERROR_RS)
        ret_resp = res.str();
}

std::string respond(playloadData &data, Record &log, Config &conf, Request &req)
{
    std::string ret_resp;
    t_state pastStatus = log.status;
    t_state pastMethod = log.method;
    if (log.status == NEW_RS)
    {
        loadExtensions(data);
        data.connection = "Close";
        if (handleResponse(log, req, conf, data))
        {
            chooseErrorHandler(conf, data);
            data.contentType = data.mime["html"];
            log.status = ERROR_RS;
        }
    }
    else if (log.status == ACTIVE_RS)
    {
        if (log.method == POST_LIST_RS)
        {
            if (postFilesList(log, data))
            {
                std::remove(log.fileName.c_str());
                data.status_code = "500";
                data.resMessage = "Internal Server Error";
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
            //remove bodyfile
        }
    }
    if (pastStatus == NEW_RS || (log.method == NONE_RS && log.status == ERROR_RS)
        || (pastMethod == POST_LIST_RS && log.method == GET_LIST_RS))
    {
        generateResponse(data, ret_resp, log, pastMethod);
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
