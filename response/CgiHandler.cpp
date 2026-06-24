#include "CgiHandler.hpp"
#include "../server/server.hpp"
#include "Response.hpp"

int cvb;

void createExecResources(CgiHandler &cgiData, playloadData &data)
{
    cvb++;
    cgiData.env = NULL;
    cgiData.env = new char*[cgiData.headers.size() + 1];
    for (size_t i = 0; i < cgiData.headers.size() + 1; i++)
        cgiData.env[i] = NULL;
    for (size_t i = 0; i < cgiData.headers.size(); i++)
    {
        cgiData.env[i] = new char[cgiData.headers[i].length() + 1];
        std::strcpy(cgiData.env[i], cgiData.headers[i].c_str());
    }
    cgiData.cgiArgs = NULL;
    cgiData.cgiArgs = new char*[3];
    cgiData.cgiArgs[0] = NULL;
    cgiData.cgiArgs[1] = NULL;
    cgiData.cgiArgs[2] = NULL;
    cgiData.cgiArgs[0] = new char[data.cgiExecutePath.length() + 1];
    // if (cvb > 50)
    //     throw std::bad_alloc();
    cgiData.cgiArgs[1] = new char[data.cgiScriptFile.length() + 1];
    std::strcpy(cgiData.cgiArgs[0], data.cgiExecutePath.c_str());
    std::strcpy(cgiData.cgiArgs[1], data.cgiScriptFile.c_str());
}

void releaseExecResources(CgiHandler &cgiData)
{
    if (cgiData.env)
    {
        for (size_t i = 0; i < cgiData.headers.size(); i++)
            delete[] cgiData.env[i];
        delete[] cgiData.env;
        cgiData.env = NULL; //know
    }
    if (cgiData.cgiArgs)
    {
        delete[] cgiData.cgiArgs[0];
        delete[] cgiData.cgiArgs[1];
        delete[] cgiData.cgiArgs;
        cgiData.cgiArgs = NULL; //know
    }
}

void closeCGI(Webserv &Web, Server &obj, std::string method)
{
    int status;
    // close(cg)
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
    kill(obj.pid, SIGKILL);
    waitpid(obj.pid, &status, WNOHANG);
}

void    returnedInternalError(CgiHandler &cgi)
{
    cgi.status = 500;
    cgi.finFlag = true;
}
void    returnedBadError(CgiHandler &cgi)
{
    cgi.status = 502;
    cgi.finFlag = true;
}

std::string toStrC(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
}

int erorrCgi(CgiHandler &cgiData)
{
    cgiData.status = 500;
    return 1;
}
CgiHandler::CgiHandler() : bodyLen(0), fd(-1), cgStatusCode(200), cgStatusName("OK"), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST), endRead(false), count(0)
{
    status = 200;
    dataReady = false;
    cgiValid = false;
    canRead = false;
    getMove = 0;
    countLenHead = 0;
    fd = -1;
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

int createPipes(Webserv &Web, Server &obj, CgiHandler &cgiData, Request req)
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

    if (req.method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(req.bodyFile.c_str(), O_RDONLY, 0644);
        if (cgiData.bodyFd < 0)//
            return (erorrCgi(cgiData));
        if (pipe(inputPipe) == -1)
            return (erorrCgi(cgiData));//
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

std::string switchData(std::string path)
{
    if (path[path.length() - 1] == '/')
        path.resize(path.length() - 1);
    size_t l = path.find_last_of('/');
    return (path.substr(0, l));
}

void createCgi(Webserv &Web, Server &obj, CgiHandler &cgiData, std::string method, playloadData &data)
{
    obj.pid = fork();
    if (obj.pid == -1)
        erorrCgi(cgiData);
    if (obj.pid == 0)
    {
        if (method == "POST")
        {
            if (dup2(obj.recs[obj.iClnt].IndexPipe[2], STDIN_FILENO) < 0)
            {
                cgiData.cgStatusCode = 500;//prblm
                throw std::exception();
            }
            close(obj.recs[obj.iClnt].IndexPipe[2]);
            close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[3]].fd);
        }

        if (dup2(obj.recs[obj.iClnt].IndexPipe[1], STDOUT_FILENO) < 0)
        {
            cgiData.cgStatusCode = 500;
            throw std::exception();
        }
        close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[0]].fd);
        close(obj.recs[obj.iClnt].IndexPipe[1]);
        if (chdir(switchData(data.realPath).c_str()) == -1)
            throw std::exception();
        execve(cgiData.cgiArgs[0], cgiData.cgiArgs, cgiData.env);
        releaseExecResources(cgiData);
        perror("error");
        throw std::exception();
        
    }
    close(obj.recs[obj.iClnt].IndexPipe[1]);
    // if (method == "POST")
    //     close(Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[3]].fd);
}

int readFromCgi(Webserv &Web, Server &obj, CgiHandler &cgiData, std::string method)
{
    static size_t n;
    if (method == "POST")
    {
        if (Web.fds[obj.recs[obj.iClnt].IndexFdsPipe[1]].revents & POLLOUT)
        {
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

int runCgi(Webserv &Web, Record &log, Server &obj, Request req)
{
    if (log.status == NEW_RS)
    {
        log.isItCgi = true;
        if (createPipes(Web, obj, log.cgiData, req))
            return (1);
        log.status = CGI_ACTIVE;
        log.method = CGI_RDWR;
        createCgi(Web, obj, log.cgiData, req.method, log.data);

        releaseExecResources(log.cgiData);
    }
    else
    {
        if (readFromCgi(Web, obj, log.cgiData, req.method))
        {
            closeCGI(Web, obj, req.method);
            return (1);
        }
        if (log.cgiData.canRead)
        {
            parseCgiResponse(log.cgiData);
            // cout << log.cgiData.cgStatusCode << " <-> " << log.cgiData.cgStatusName << std::endl;
            log.cgiData.canRead = false;
        }
    }
    // std::cout << obj.pid << std::endl;
    return 0;
}

void getCgiEnv(playloadData &data, Request req, CgiHandler &cgiData, std::vector<int> &listenPorts) // put them on the class so you'll have refer
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
    cgiData.headers.push_back("GATEWAY_INTERFACE=CGI/1.1");
    cgiData.headers.push_back(pathInfo);
    cgiData.headers.push_back(query);
    cgiData.headers.push_back("REQUEST_METHOD=" + req.method);
    cgiData.headers.push_back("REDIRECT_STATUS=200");
    cgiData.headers.push_back("SCRIPT_NAME=" + data.cgiFile);
    cgiData.headers.push_back("SCRIPT_FILENAME=" + data.cgiScriptFile);
    cgiData.headers.push_back("SERVER_PORT=" + port); // you should get the working server port
    cgiData.headers.push_back("SERVER_PROTOCOL=HTTP/1.0");

    std::map<std::string, std::string>::iterator iter;
    for (iter = req.headers.begin(); iter != req.headers.end(); iter++)
    {
        if (iter->first == "content-lenght")
            cgiData.headers.push_back("CONTENT_LENGTH=" + iter->second);
        else if (iter->second == "content-type")
            cgiData.headers.push_back("CONTENT_TYPE=" + iter->second);
        else
            cgiData.headers.push_back("HTTP_" + normalizeKey(iter->first) + "=" + iter->second);
    }

    try
    {
        createExecResources(cgiData, data);
    }
    catch (std::exception &e)
    {
        releaseExecResources(cgiData);
        throw std::bad_alloc();
    }

    // int i = -1;
    // while (cgiData.env[++i])
    //     std::cout << cgiData.env[i] << std::endl;
    // std::cout << cgiData.cgiArgs[0] << std::endl;
    // std::cout << cgiData.cgiArgs[1] << std::endl;
}

int cgiResponseHandler(Record &log, CgiHandler &cgiData, playloadData &data)
{
    if (cgiData.status == 500 || cgiData.status == 502)
    {
        data.statusCode = cgiData.status;
        return 1;
    }
    if (data.table.count(cgiData.cgStatusCode))
    {
        if (data.table[cgiData.cgStatusCode] != cgiData.cgStatusName)
        {
            data.statusCode = 502;
            return 1;
        }
        data.statusCode = cgiData.cgStatusCode;
    }
    else
    {
        data.statusCode = 502;
        return 1;
    }
    log.fd = open(cgiData.cgBodyFileName.c_str(), O_RDONLY);
    struct stat info;
    if (log.fd == -1 || stat(cgiData.cgBodyFileName.c_str(), &info) == -1 )
    {
        data.statusCode = 500;
        data.statusName = "Internal Server Error";
        return 1;
    }
    data.bodySize = info.st_size;
    log.status = CLOSED_RS;
    if (getFile(log))
        return 1;
    return 0;
}

int cgiProcess(Webserv &Web, Record &log, Server &obj, Request req)
{
    if (runCgi(Web, log, obj, req))
    {
        log.status = ERROR_RS;
    }
    if (log.cgiData.dataReady == true || log.status == ERROR_RS)
    {
        if (log.cgiData.fd != -1)
            close(log.cgiData.fd), log.cgiData.fd = -1;
        if (log.cgiData.dataReady)
        {
            closeCGI(Web, obj, req.method);
        }
        if (cgiResponseHandler(log, log.cgiData, log.data))
            return 1;
        else
            log.cgiData.cgiValid = true;
    }
    return 0;
}

int cgiHandler(Webserv &Web, Record &log, Server &obj, Request req, std::vector<int> &listenPorts)
{
    std::vector<char *> exFile;
    std::vector<char *> env;
    std::vector<std::string> headers;
    if (log.status == NEW_RS)
        log.cgiData.dataReady = false;
    getCgiEnv(log.data, req, log.cgiData, listenPorts);
    if (cgiProcess(Web, log, obj, req))
        return 1;
    return 0;
}
