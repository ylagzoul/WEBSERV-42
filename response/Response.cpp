#include "../server/server.hpp"

void generateResponse(Request &req, payloadData &data, std::string &ret_resp)
{
    (void)ret_resp;
    std::stringstream res;
    res << req.httpVersion << ' ' << data.status_code << ' ' << data.message + "\r\n";
    res << "Content-Length: " << data.bodySize << "\r\n";
    res << "Content-Type: " << data.contentType << "\r\n";
    res << "Connection: " << data.connection << "\r\n";
    res << "\r\n";
    // res << data.bodyStr;
    ret_resp = res.str();
}

void errorPage(payloadData &data)
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
                 + data.message + "</h1>\n";
    data.bodyStr += 
            "<hr>"
        "</body>\n"
    "</html>\n";
}

int getStatusCode(Request &req, payloadData &data)
{
    if (req.status == 400)
    {
        data.status_code = "400";
        data.message = "Bad Request";
        return 1;
    }
    else if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
    {
        data.status_code = "405";
        data.message = "Method Not Allowed";
        return 1;
    }
    else
    {
        data.status_code = "200";
        data.message = "OK";
    }
    return 0;
}

void loadExtensions(payloadData &data)
{
    data.mime["html"] = "text/html";
    data.mime["css"] = "text/css";
    data.mime["xml"] = "text/xml";
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

void resolvePath(payloadData &data, Config &conf, std::string &path)
{
    std::stringstream fullPath(conf.root + path);
    std::vector<std::string> realNames;
    std::string tmp;

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
        data.realPath += '/' + realNames[i];
}

void listFiles(payloadData &data)
{
    DIR *dir;
    dir = opendir(data.realPath.c_str());
    if (!dir)
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
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

int autoIndex(Config conf, std::string reqPath)
{
    for(size_t i = 0; i < conf.location.size(); i++)
    {
        if (conf.location[i].conf.server_name == reqPath)
        {
            if (conf.location[i].conf.autoIndex)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

int validateFile(payloadData &data, Config &conf, std::string reqPath)
{
    struct stat info;

    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.status_code = "404";
        data.message = "Not Found";
        return 1;
    }
    else if (access(data.realPath.c_str(), R_OK) == -1)
    {
        data.status_code = "403";
        data.message = "Forbidden";
        return 1;
    }
    stat(data.realPath.c_str(), &info);
    if (S_ISDIR(info.st_mode))
    {
        if (autoIndex(conf, reqPath))
            listFiles(data);
        else
        {
            data.status_code = "403";
            data.message = "Forbidden";
        }
        return 1;
    }
    else
        data.bodySize = info.st_size;
    return 0;
}

int handleDelete(payloadData &data)
{
    struct stat info;

    if (access(data.realPath.c_str(), F_OK) == -1)
    {
        data.status_code = "404";
        data.message = "Not Found";
        return 1;
    }
    stat(data.realPath.c_str(), &info);
    if (S_ISDIR(info.st_mode))
    {
        data.status_code = "403";
        data.message = "Forbidden";
        return 1;
    }
    else if (access(data.realPath.c_str(), W_OK) == -1)
    {
        data.status_code = "403";
        data.message = "Forbidden";
        return 1;
    }
    else if (remove(data.realPath.c_str()))
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
        return 1;
    }
    data.status_code = "204";
    data.message = "No Content";
    return 0;
}

int handlePost(Record log, payloadData &data)
{
    log.fd += 0;
    data.bodySize += 0;
    data.bodySize += 0;
    return 0;
}

int getFileData(Record &log, payloadData &data)
{
    const int size = 8000;
    char buffer[size];
    int b = 0;
    b = read(log.fd, buffer, sizeof(buffer));
    if (b == -1)
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
        return 1;
    }
    log.buffer = buffer;
    if (b < 8000)
    {
        log.status = CLOSED_RS;
        close(log.fd);
    }
    else
    {
        log.status = ACTIVE_RS;
    }
    return 0;
}

int handleGet(Record &log, payloadData &data, Config &conf, std::string reqPath)
{
    int check;
    std::string extension;
    check = validateFile(data, conf ,reqPath);
    if (data.status_code != "200")
        return 1;
    if (check)
        return 0;
    log.fd = open(data.realPath.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
        log.status = CLOSED_RS;
        return 1;
    }
    size_t pos = data.realPath.find_last_of('.');
    data.contentType = "application/octet-stream";
    if (pos != std::string::npos)
    {
        extension = data.realPath.substr(pos + 1);
        if (data.mime.count(extension))
            data.contentType = data.mime[extension];
    }
    getFileData(log, data);
    return 0;
}

int handleResponse(Record &log, Request &req, Config &conf, payloadData &data)
{    
    if (getStatusCode(req, data))
    {
        errorPage(data);
        return 1;
    }
    if (req.headers["Connection"] == "Keep-Alive")
        data.connection = "Keep-Alive";

    resolvePath(data, conf, req.path);
    
    //!!!! check get and post and delete if they are allowed in the locations in config !!!!
    if (req.method == "GET" && handleGet(log, data, conf, req.path))
    {
        errorPage(data);
        return 1;
    }
    else if (req.method == "DELETE" && handleDelete(data))
    {
        errorPage(data);
        return 1;
    }
    else if (req.method == "POST" && handlePost(log, data))
    {
        errorPage(data);
        return 1;
    }
    return 0;
}

std::string respond(Record &log, Server serv, Config &conf)
{
    
    std::string ret_resp;
    payloadData data;
    // //////////////////tmp
    Request req;
    (void)serv;

    req.status = 0;
    req.method = "GET";
    req.path = "file";
    req.httpVersion = "HTTP/1.0"; 
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";
    t_state check = log.status;
    // //////////////////
    
    if (log.status == NEW_RS)
    {
        loadExtensions(data);
        data.connection = "close";
        if (handleResponse(log, req, conf, data))
        {
            data.contentType = data.mime["html"];        
            data.bodySize = data.bodyStr.length();
            log.status = ERROR_RS;
        }
    }
    else if (log.status == ACTIVE_RS)
    {
        if (log.method == GET_RS)
        {
            if (getFileData(log, data))
            {
                errorPage(data);
                data.bodySize = data.bodyStr.length();
                log.status = ERROR_RS;
            }
        }
        // else if (POST_RS)
        // {
        //     // Behaviour
        // }
    }
    if (check == NEW_RS || (check == NEW_RS && log.status == ERROR_RS))
    {
        generateResponse(req, data, ret_resp);
        if (log.status == ERROR_RS)
            ret_resp += data.bodyStr;
        else
            ret_resp += log.buffer;
    }
    else if (log.status == ERROR_RS)
        ret_resp = std::string();
    else
        ret_resp = log.buffer;
    // ----------------------------------Teeeest--------------------------------------

    // int fd = open("/home/abouhki/Desktop/webserv_p/response/body.html", O_WRONLY);
    // write(fd, ret_resp.c_str(), ret_resp.length());
    // -------------------------------------------------------------------------------

    return (ret_resp);
}
