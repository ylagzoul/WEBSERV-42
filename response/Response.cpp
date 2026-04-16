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
    res << data.bodyStr;
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

int handlePost(payloadData &data)
{
    data.bodySize += 0;
    return 0;
}

int handleGet(payloadData &data, Config &conf, std::string reqPath)
{
    int fd;
    int check;
    std::string extension;
    check = validateFile(data, conf ,reqPath);
    if (data.status_code != "200")
        return 1;
    if (check)
        return 0;
    fd = open(data.realPath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
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
    int size = 8192;
    char buffer[size];
    // neeeed the serv part to handle get properly
    int b = 0;
    while ((b = read(fd, buffer, sizeof(buffer))) > 0)
        data.bodyStr.append(buffer, b);
    if (b == -1)
    {
        data.status_code = "500";
        data.message = "Internal Server Error";
        return 1;
    }
    return 0;
}

int handleResponse(Request &req, Config &conf, payloadData &data)
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
    if (req.method == "GET" && handleGet(data, conf, req.path))
    {
        errorPage(data);
        return 1;
    }
    else if (req.method == "DELETE" && handleDelete(data))
    {
        errorPage(data);
        return 1;
    }
    else if (req.method == "POST" && handlePost(data))
    {
        errorPage(data);
        return 1;
    }
    return 0;
}

std::string respond(Server serv, Config &conf)
{
    
    std::string ret_resp;
    payloadData data;
    // //////////////////tmp
    Request req;
    (void)serv;

    req.status = 0;
    req.method = "GET";
    req.path = "response";
    req.httpVersion = "HTTP/1.0"; 
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";
    // //////////////////


    loadExtensions(data);
    data.connection = "close";
    if (handleResponse(req, conf, data))
    {
        data.contentType = data.mime["html"];
        //behavior
    }
    data.bodySize = data.bodyStr.length();
    generateResponse(req, data, ret_resp);
    // ----------------------------------Teeeest--------------------------------------

    // int fd = open("/home/abouhki/Desktop/webserv_p/response/body.html", O_WRONLY);
    // write(fd, ret_resp.c_str(), ret_resp.length());
    // -------------------------------------------------------------------------------

    return (ret_resp);
}
