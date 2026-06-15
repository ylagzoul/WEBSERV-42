#include "CgiHandler.hpp"
#include "../server/server.hpp"
#include "Response.hpp"

void closeCGI(Webserv &Web, Server &obj, std::string method)
{
    if (method == "POST")
    {
        close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[1]].fd);
        Web.fds.erase(Web.fds.begin() + obj.recs[obj.iClnt].IndexFdsPipe[1]);
        obj.req.erase(obj.req.begin() + obj.recs[obj.iClnt].IndexPipe[3]);
        obj.recs.erase(obj.recs.begin() + obj.recs[obj.iClnt].IndexPipe[3]);
        obj.data.erase(obj.data.begin() + obj.recs[obj.iClnt].IndexPipe[3]);
    }
    close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[0]].fd);
    Web.fds.erase(Web.fds.begin() + obj.recs[obj.iClnt].IndexFdsPipe[0]);
    obj.req.erase(obj.req.begin() + obj.recs[obj.iClnt].IndexPipe[0]);
    obj.recs.erase(obj.recs.begin() + obj.recs[obj.iClnt].IndexPipe[0]);
    obj.data.erase(obj.data.begin() + obj.recs[obj.iClnt].IndexPipe[0]);
}

std::string toStrC(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
}

int erorrCgi(CgiHandler &cgiData)
{
    cgiData.cgStatusCode = 500;
    cgiData.cgStatusName = "Internal Server Error";
    return 1;
}
CgiHandler::CgiHandler() : bodyLen(0), fd(-1), cgStatusCode(200), cgStatusName("OK"), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST), endRead(false), count(0)
{
    dataReady = false;
    cgiValid = false;
    canRead = false;
};

std::string normalizeKey(std::string key)
{
    for (size_t i = 0; i < key.length(); i++)
    {
        key[i] = toupper(key[i]);
        if (key[i] == '-')
            key[i] = '_';
    }
    return key;
}

int createPipes(Webserv &Web, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (erorrCgi(cgiData));

    pollfd client_poll;
    client_poll.fd = outputPipe[0];
    client_poll.events = POLLIN;
    client_poll.revents = 0;
    Web.fds.push_back(client_poll);
    obj.recs[obj.iClnt].IndexFdsPipe.push_back(Web.fds.size() - 1);
    obj.recs.push_back(Record());
    obj.req.push_back(Request());
    obj.data.push_back(Data());
    obj.recs[obj.iClnt].IndexPipe.push_back(obj.recs.size() - 1);

    obj.recs[obj.iClnt].IndexPipe.push_back(outputPipe[1]);
    std::cout << "   : " << obj.recs[obj.iClnt].IndexPipe[0] << std::endl;

    if (method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(bodyFile.c_str(), O_RDONLY, 0644);
        if (cgiData.bodyFd < 0)
            return (erorrCgi(cgiData));
        if (pipe(inputPipe) == -1)
            return (erorrCgi(cgiData));

        obj.recs[obj.iClnt].IndexPipe.push_back(inputPipe[0]);

        pollfd client_poll;
        client_poll.fd = inputPipe[1];
        client_poll.events = POLLOUT;
        client_poll.revents = 0;
        Web.fds.push_back(client_poll);
        obj.recs[obj.iClnt].IndexFdsPipe.push_back(Web.fds.size() - 1);
        obj.recs.push_back(Record());
        obj.req.push_back(Request());
        obj.data.push_back(Data());
        obj.recs[obj.iClnt].IndexPipe.push_back(obj.recs.size() - 1);
    }
    return (0);
}

int createCgi(Webserv &Web, Server &obj, CgiHandler &cgiData, std::string method)
{
    pid_t pid = fork();

    if (pid == -1)
        return (erorrCgi(cgiData));
    if (pid == 0)
    {
        if (method == "POST")
        {
            if (dup2(obj.recs[obj.iClnt].IndexPipe[2], STDIN_FILENO) < 0)
                return (erorrCgi(cgiData));
            close(obj.recs[obj.iClnt].IndexPipe[2]);
            close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[1]].fd);
        }

        if (dup2(obj.recs[obj.iClnt].IndexPipe[1], STDOUT_FILENO) < 0)
            return (erorrCgi(cgiData));
        close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[0]].fd);
        close(STDIN_FILENO);
        close(obj.recs[obj.iClnt].IndexPipe[1]);

        execve(cgiData.path.c_str(), cgiData.executeablePath, cgiData.env);
        exit(1);
    }

    close(obj.recs[obj.iClnt].IndexPipe[1]);
    return (0);
}

int readFromCgi(Webserv &Web, Server &obj, CgiHandler &cgiData, std::string method)
{
    static size_t n;
    if (method == "POST")
    {
        if (Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[1]].revents & POLLOUT)
        {
            // std::cout << "post writed form cgi if\n\n";
            int bytes_r;
            char buffer[SIZESR];
            bytes_r = read(cgiData.bodyFd, buffer, SIZESR);
            if (bytes_r < 0)
                return (erorrCgi(cgiData));
            int k = 0;
            if ((k = write(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[1]].fd, buffer, bytes_r)) < 0)
                return (erorrCgi(cgiData));
            n += k; 
        }
    }
    if (Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[0]].revents & (POLLIN | POLLHUP))
    {
        char buffer[SIZESR];
        int bytes = read(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[0]].fd, buffer, SIZESR - 1);

        if (bytes < 0)
            return (erorrCgi(cgiData));
        else if (bytes == 0)
        {
            cgiData.dataReady = true;
            cgiData.endRead = true;
            std::cout << "close CGI     ..\n";
            closeCGI(Web, obj, method); // 7ala
        }
        else if (bytes > 0)
        {
            cgiData.canRead = true;
            buffer[bytes] = '\0';
            cgiData.cgBuff.append(buffer, bytes);
        }
    }

    return (0);
}

int runCgi(Webserv &Web, Record &log, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    if (log.status == NEW_RS)
    {
        log.isItCgi = true;
        if (createPipes(Web, obj, cgiData, method, bodyFile))
            return (1);
        log.status = CGI_ACTIVE;
        log.method = CGI_RDWR;
        if (createCgi(Web, obj, cgiData, method))
            return (1);
    }
    else
    {
        if (readFromCgi(Web, obj, cgiData, method))
            return (1);
        if (cgiData.canRead) /* CHEEEECK /!\*/
        {

            parseCgiResponse(cgiData);
            cgiData.canRead = false;
        }
    }
    return 0;
}

void getCgiEnv(std::vector<std::string> &headers, std::vector<char *> &env, std::vector<char *> &exFile, playloadData &data, Request &req, CgiHandler &cgiData, std::vector<int> &listenPorts) // put them on the class so you'll have refer
{
    std::string pathInfo = "PATH_INFO=";
    std::string query = "QUERY_STRING=";
    size_t k = data.cgiInfo.find('?');

    std::stringstream ss;
    ss << listenPorts[0];
    std::string port = ss.str();
    if (k != std::string::npos)
    {
        pathInfo += data.cgiInfo.substr(0, k);
        query += data.cgiInfo.substr(k + 1);
    }
    else
        pathInfo += data.cgiInfo;
    // headers.push_back("AUTH")!!
    headers.push_back("GATEWAY_INTERFACE=CGI/1.1");
    headers.push_back(pathInfo);
    headers.push_back(query);
    headers.push_back("REQUEST_METHOD=" + req.method);
    headers.push_back("REDIRECT_STATUS=200");
    headers.push_back("SCRIPT_NAME=" + data.cgiFile);
    headers.push_back("SCRIPT_FILENAME=" + data.cgiScriptFile);
    headers.push_back("SERVER_PORT=" + port); // you should get the working server port
    headers.push_back("SERVER_PROTOCOL=HTTP/1.0");

    std::map<std::string, std::string>::iterator iter;
    for (iter = req.headers.begin(); iter != req.headers.end(); iter++)
    {
        if (iter->first == "content-lenght")
            headers.push_back("CONTENT_LENGTH=" + iter->second);
        else if (iter->second == "content-type")
            headers.push_back("CONTENT_TYPE=" + iter->second);
        else
            headers.push_back("HTTP_" + normalizeKey(iter->first) + "=" + iter->second);
    }
    //                      check                     //
    for (size_t i = 0; i < headers.size(); i++)
        env.push_back((char *)headers[i].c_str());
    env.push_back(NULL);
    cgiData.env = &env[0];

    exFile.push_back((char *)data.cgiExecutePath.c_str());
    exFile.push_back(NULL);
    cgiData.executeablePath = &exFile[0];
    // std::cout << "-------------->" << *cgiData.executeablePath << std::endl;

    cgiData.path = data.cgiScriptFile;

    // int i = -1;
    // while (cgiData.env[++i])
    //     std::cout << env[i] << std::endl;
}

int cgiResponseHandler(Record &log, CgiHandler &cgiData, playloadData &data)
{
    if (cgiData.cgStatusCode != 200)
    {
        std::stringstream ss;
        ss << cgiData.cgStatusCode;
        data.statusName = cgiData.cgStatusName;
        data.statusCode = ss.str();
        return 1;
    }
    log.fd = open(cgiData.cgBodyFileName.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.statusCode = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
    log.status = CLOSED_RS;
    if (getFile(log, data))
        return 1;
    return 0;
}
int cgiProcess(Webserv &Web, CgiHandler &cgiData, Record &log, Server &obj, playloadData &data, Request &req)
{
    if (runCgi(Web, log, obj, cgiData, req.method, req.bodyPath))
    {
        std::cout << "ERROR_RS\n";
        log.status = ERROR_RS;
    }
    if (cgiData.dataReady == true || log.status == ERROR_RS)
    {
        std::cerr << "to true\n";
        if (cgiResponseHandler(log, cgiData, data))
            return 1;
        else
            cgiData.cgiValid = true;
    }
    return 0;
}

int cgiHandler(Webserv &Web, Record &log, CgiHandler &cgiData, Server &obj, playloadData &data, Request &req, std::vector<int> &listenPorts)
{
    std::vector<char *> exFile;
    std::vector<char *> env;
    std::vector<std::string> headers;
    if (log.status == NEW_RS)
        cgiData.dataReady = false;
    getCgiEnv(headers, env, exFile, data, req, cgiData, listenPorts);
    if (cgiProcess(Web, cgiData, log, obj, data, req))
        return 1;

    return 0;
}
