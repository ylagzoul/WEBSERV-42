#include "../server/server.hpp"

void generateResponse(Request &req, playloadData &data, std::string &ret_resp)
{
    std::stringstream res;
    data.bodySize = data.bodyStr.length();
    res << req.httpVersion << ' ' << data.status_code << ' ' << data.resMessage + "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "\r\n";
    res << data.bodyStr;
    ret_resp = res.str();
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
    data.bodyStr += "<h1 style=\"text-align: center;\">" + data.status_code + ' '
                 + data.resMessage + "</h1>\n";
    data.bodyStr += 
            "<hr>"
        "</body>\n"
    "</html>\n";
}

int getStatusCode(Request &req, playloadData &data)
{
    if (req.status == 400)
    {
        data.status_code = "400";
        data.resMessage = "Bad Request";
        return 1;
    }
    else if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
    {
        data.status_code = "405";
        data.resMessage = "Method Not Allowed";
        return 1;
    }
    else
    {
        data.status_code = "200";
        data.resMessage = "OK";
    }
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

// int autoIndex(Config conf, std::string reqPath)
// {
//     for(size_t i = 0; i < conf.location.size(); i++)
//     {
//         if (conf.location[i].conf.location == reqPath)
//         {
//             if (conf.location[i].conf.autoIndex)
//                 return 1;
//             else
//                 return 0;
//         }
//     }
//     return 0;
// }

void getContentType(playloadData &data)
{
    std::string extension;
    size_t pos = data.realPath.find_last_of('.');
    data.contentType = "application/octet-stream";
    if (pos != std::string::npos)
    {
        extension = data.realPath.substr(pos + 1);
        if (data.mime.count(extension))
            data.contentType = data.mime[extension];
    }
}

int handleDelete(playloadData &data)
{
    struct stat info;

    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.status_code = "404";
        data.resMessage = "Not Found";
        return 1;
    }
    stat(data.realPath.c_str(), &info);//fail
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
    else if (remove(data.realPath.c_str()))
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return 1;
    }
    data.status_code = "204";
    data.resMessage = "No Content";
    return 0;
}

std::string getRandomName(Config &conf)
{
    time_t tStamp = time(NULL);
    struct tm *currentTime = localtime(&tStamp);
    char arr[9];

    strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    randName += '_';
    randName += (conf.count + 48);
    conf.count++;
    return randName;
}

int postBody(Record &log, playloadData &data)
{
    const int size = 61;
    char buff[size];
    int bytes = read(log.fileFd, buff, sizeof(buff));
    if (bytes == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        close(log.fd);
        close(log.fileFd);
        return 1;
    }
    write(log.fd, buff, bytes);
    if (bytes < size)
    {
        log.status = CLOSED_RS;
        close(log.fd);
        close(log.fileFd);
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
            log.fd = open(tmpStr.c_str(), O_RDWR | O_CREAT, 0666);
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
        log.fd = open(data.realPath.c_str(), O_RDWR | O_CREAT, 0666);
        if (log.fd == -1)
        {
            data.status_code = "403";
            data.resMessage = "Forbidden";
            return 1;
        }
        log.fileFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (log.fileFd == -1)
        {
            data.status_code = "500";
            data.resMessage = "Internal Server Error";
            return 1;
        }
        if (postBody(log, data))
            return 1;
        getContentType(data);
    }
    data.status_code = "201";
    data.resMessage = "Created";
    return 0;
}


void listFiles(playloadData &data)
{
    DIR *dir;
    dir = opendir(data.realPath.c_str());
    if (!dir)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        return ;
    }
    struct dirent *rdir;
    std::string tmp;
    data.bodyStr =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
        "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>File List</title>\n"
        "</head>\n"
        "<body>\n"
            "<h1>File List</h1>\n"
            "<a href=\"../\">../</a><br>\n";
    while ((rdir = readdir(dir)))
    {
        tmp = rdir->d_name;
        if (tmp == "." || tmp == "..")
            continue ;
        data.bodyStr += "<a href=\"" + tmp + "\">" + tmp + "</a><br>\n";
    }
    data.bodyStr += 
        "</body>\n"
    "</html>\n";
    data.contentType = data.mime["html"];
    closedir(dir);
}

int getFileData(Record &log, playloadData &data)
{
    const int size = 8192;
    char buffer[size + 1];
    int bytes = 0;
    bytes = read(log.fd, buffer, size); // sizeof(buffer) - 1
    if (bytes == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        close(log.fd);
        return 1;
    }
    buffer[bytes] = '\0';
    log.buffer = buffer;
    if (bytes < size)
    {
        log.status = CLOSED_RS;
        close(log.fd);
    }
    else
    {
        log.status = ACTIVE_RS;
        log.method = GET_RS;
    }
    return 0;
}

int validateFile(playloadData &data, Config &conf)
{
    struct stat info;
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
    stat(data.realPath.c_str(), &info);
    if (S_ISDIR(info.st_mode))
    {

        if (conf.location[data.locationIndex].conf.autoIndex)
            listFiles(data);
        else
        {
            data.status_code = "403";
            data.resMessage = "Forbidden";
        }
        return 1;
    }
    else
        data.bodySize = info.st_size;
    return 0;
}

int handleGet(Record &log, playloadData &data, Config &conf)
{
    int check;
    check = validateFile(data, conf);
    if (data.status_code != "200")
        return 1;
    if (check)
        return 0;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.status_code = "500";
        data.resMessage = "Internal Server Error";
        log.status = CLOSED_RS;
        return 1;
    }
    getFileData(log, data);
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
        data.location = "/";
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
    for(size_t i = 0; i < conf.location.size(); i++)
    {
        if (conf.location[i].location_name == data.location)
        {
            if (!conf.location[i].conf.error_page.empty())
                data.errorPages = conf.location[i].conf.error_page;
            else
                data.errorPages = conf.error_page;
            data.locationMethods = conf.location[i].conf.methods;
            data.locationIndex = i;
            data.locationRoot = conf.location[i].conf.root;
            data.locationUpload = conf.uploadPath;//conf.location[i].conf.uploadPath
            break ;
        }
    }
    if (data.locationMethods.empty())
    {
        data.locationMethods = conf.methods;
    }
}
 
int checkErrorFile(playloadData &data, Config &conf)
{
    int code = atoi(data.status_code.c_str());
    std::string file = conf.location[data.locationIndex].conf.root;
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
    stat(file.c_str(), &info);

    if (S_ISDIR(info.st_mode))
        return 1;
    if (info.st_size > 64 * 1024)
        return 1;
    data.bodySize = info.st_size;
    int fd = open(file.c_str(), O_RDONLY);
    if (fd == -1)
        return 1;
    char buff[8192];
    int bytes = 0;
    while ((bytes = read(fd, buff, sizeof(buff) - 1)) > 0)
        data.bodyStr.append(buff, bytes);
    if (bytes < 0)
        return 1;
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
    std::stringstream fullPath(data.locationRoot + data.uri);
    std::vector<std::string> realNames;
    std::string tmp;
    // std::string tmpPath;
    // struct stat info;
    // stat(data.realPath.c_str(), &info);// cgi
    while (std::getline(fullPath, tmp, '/'))
    {
        if (tmp == "" || tmp == ".")
            continue;
        if (tmp == "..")
        {
            if (!realNames.empty())
                realNames.pop_back();
        }
        else
            realNames.push_back(tmp);
    }

    for (size_t i = 0; i < realNames.size(); i++)
    {
        // tmpPath += '/' + realNames[i];
        data.realPath += '/' + realNames[i];
    }
    data.uploadOn = 1;
    if (data.locationUpload.empty())
        return ;
    if (data.locationRoot[data.locationRoot.length() - 1] != '/'
        && data.locationUpload[0] != '/')
        data.locationUpload.insert(0, "/");
    std::string rootUploadDir = data.locationRoot + data.locationUpload;
    // std::string compare = data.realPath.substr(0, rootUploadDir.length());
    // if (compare == rootUploadDir)
    //     data.uploadOn = 1;
}
// std::cout << data.realPath << "\n\n";
    // std::cout << rootUploadDir << "\n\n"; //debug!!
    // std::cout << compare << "\n\n";

int handleResponse(Record &log, Request &req, Config &conf, playloadData &data)
{
    findLongestMatch(data, conf, req);
    getImportantData(conf, data);
    resolvePath(data, conf);

    if (getStatusCode(req, data))
    {
        chooseErrorHandler(conf, data);
        return 1;
    }
    if (req.headers["Connection"] == "Keep-Alive")
        data.connection = "Keep-Alive";

    if (req.method == "GET") 
    {
        if (!data.locationMethods["GET"])
        {
            data.status_code = "405";
            data.resMessage = "Method Not Allowed";
            chooseErrorHandler(conf, data);
            return 1;
        }
        else if (handleGet(log, data, conf))
        {
            chooseErrorHandler(conf, data);
            return 1;
        }
    }
    else if (req.method == "POST") 
    {
        if (!data.locationMethods["POST"])
        {
            data.status_code = "405";
            data.resMessage = "Method Not Allowed";
            chooseErrorHandler(conf, data);
            return 1;
        }
        else if (handlePost(log, data, req, conf))
        {
            chooseErrorHandler(conf, data);
            return 1;
        }
    }
    else if (req.method == "DELETE") 
    {
        if (!data.locationMethods["DELETE"])
        {
            data.status_code = "405";
            data.resMessage = "Method Not Allowed";
            chooseErrorHandler(conf, data);
            return 1;
        }
        else if (handleDelete(data))
        {
            chooseErrorHandler(conf, data);
            return 1;
        }
    }
    return 0;
}

std::string respond(Record &log, Server serv, Config &conf, Request &req)
{
    std::string ret_resp;
    playloadData data;
    (void)serv;
    t_state check = log.status;
    if (log.status == NEW_RS)
    {
        loadExtensions(data);
        data.connection = "close";
        if (handleResponse(log, req, conf, data))
        {
            data.contentType = data.mime["html"];        
            // data.bodySize = data.bodyStr.length();
            log.status = ERROR_RS;
        }
    }
    else if (log.status == ACTIVE_RS)
    {
        if (log.method == GET_RS)
        {
            if (getFileData(log, data))
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
        }
    }
    if (check == NEW_RS)
    {
        generateResponse(req, data, ret_resp);
        if (log.status == ERROR_RS)
            ret_resp += data.bodyStr;
        else
            ret_resp += log.buffer;
    }
    else if (log.status == ERROR_RS)
        ret_resp.clear();
    else
        ret_resp = log.buffer;
    if (log.status == CLOSED_RS || log.status == ERROR_RS)
        log.buffer.clear();
    return (ret_resp);
}
