#include "CgiHandler.hpp"
#include "../server/server.hpp"
#include "Response.hpp"

// TODO Remove int cvb;

void createExecResources(CgiHandler &cgiData, playloadData &data)
{
    // cvb++; // TODO Remove 
    cgiData.env = NULL;
    cgiData.env = new char *[cgiData.headers.size() + 1];
    for (size_t i = 0; i < cgiData.headers.size() + 1; i++)
        cgiData.env[i] = NULL;
    for (size_t i = 0; i < cgiData.headers.size(); i++)
    {
        cgiData.env[i] = new char[cgiData.headers[i].length() + 1];
        std::strcpy(cgiData.env[i], cgiData.headers[i].c_str());
    }
    cgiData.cgiArgs = NULL;
    cgiData.cgiArgs = new char *[3];
    cgiData.cgiArgs[0] = NULL;
    cgiData.cgiArgs[1] = NULL;
    cgiData.cgiArgs[2] = NULL;
    cgiData.cgiArgs[0] = new char[data.cgiExecutePath.length() + 1];
    // if (cvb > 50)// TODO Remove 
    //     throw std::bad_alloc();// TODO Remove 
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
        cgiData.env = NULL; // know// TODO Remove 
    }
    if (cgiData.cgiArgs)
    {
        delete[] cgiData.cgiArgs[0];
        delete[] cgiData.cgiArgs[1];
        delete[] cgiData.cgiArgs;
        cgiData.cgiArgs = NULL; // know
    }
}

void closeCGI(Webserv &Web, CgiHandler &cgiData, std::string method, size_t &indexClnt)
{
    if (method == "POST")
    {
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd);
        Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[2]] = "close";
        Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[3]] = "close";
        Web.reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[2]);
        Web.reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[3]);
    }
    
    close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
    Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[0]] = "close";
    Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[1]] = "close";
    Web.reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[0]);
    Web.reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[1]);

    kill(cgiData.pid, SIGKILL);
}

void returnedInternalError(CgiHandler &cgi)
{
    cgi.status = 500;
    cgi.finFlag = true;
}
void returnedBadError(CgiHandler &cgi)
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
CgiHandler::CgiHandler() : bodyLen(0), fd(-1), cgStatusCode(200), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST), endRead(false), count(0)
{
    status = 200;
    bodyType = 0;
    dataReady = false;
    cgiValid = false;
    canRead = false;
    getMove = 0;
    countLenHead = 0;
    chunkSize = 0;
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

void addFdPollfd(Webserv &Web, size_t &indexClnt, size_t fdPipe, int poll)
{
    pollfd client_poll;
    client_poll.fd = fdPipe;
    client_poll.events = poll;
    client_poll.revents = 0;
    if (Web.reuseFd.size() == 0)
    {
        Web.fds.push_back(client_poll);
        Web.typeFd[Web.fds.size() - 1] = "pipe";
        Web.statusFd[Web.fds.size() - 1] = "active";
        Web.server[Web.s].serverPort[Web.t].index_Client.push_back(Web.fds.size() - 1);
        Web.recs.push_back(Record());
        Web.req.push_back(Request());
        Web.data.push_back(Data());
        Web.recs[indexClnt].IndexFdsPipe.push_back(Web.fds.size() - 1);
    }
    else
    {
        Web.fds[Web.reuseFd[0]] = client_poll;
        Web.typeFd[Web.reuseFd[0]] = "pipe";
        Web.statusFd[Web.reuseFd[0]] = "active";
        Web.req[Web.reuseFd[0]]  = Request();
        Web.recs[Web.reuseFd[0]] = Record();
        Web.data[Web.reuseFd[0]] = Data();
        Web.recs[indexClnt].IndexFdsPipe.push_back(Web.reuseFd[0]);
        Web.reuseFd.erase(Web.reuseFd.begin() + 0);
    }
    fcntl(fdPipe, F_SETFL, O_NONBLOCK);
}

int createPipes(Webserv &Web, CgiHandler &cgiData, Request req, size_t &indexClnt)
{
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (erorrCgi(cgiData));

    addFdPollfd(Web, indexClnt , outputPipe[0],POLLIN);
    addFdPollfd(Web, indexClnt , outputPipe[1],POLLOUT);

    if (req.method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(req.bodyFile.c_str(), O_RDONLY, 0644); // boha wach close fd open
        if (cgiData.bodyFd < 0)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            return (erorrCgi(cgiData));
        }

        if (pipe(inputPipe) == -1)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            return (erorrCgi(cgiData));
        }
        addFdPollfd(Web, indexClnt , inputPipe[0],POLLIN);
        addFdPollfd(Web, indexClnt , inputPipe[1],POLLOUT);
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

void createCgi(Webserv &Web, CgiHandler &cgiData, std::string method, playloadData &data, size_t &indexClnt)
{
    cgiData.pid = fork();
    if (cgiData.pid == -1)
    {
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
        closeCGI(Web, Web.recs[indexClnt].cgiData, Web.req[indexClnt].method, indexClnt);
        erorrCgi(cgiData);
        throw std::exception();
    }
    if (cgiData.pid == 0)
    {
        if (method == "POST")
        {
            if (dup2(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd, STDIN_FILENO) < 0)
            {
                close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd);
                close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
                cgiData.cgStatusCode = 500; // prblm
                throw std::exception();
            }
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
        }

        if (dup2(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd, STDOUT_FILENO) < 0)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            cgiData.cgStatusCode = 500;
            throw std::exception();
        }
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
        if (chdir(switchData(data.realPath).c_str()) == -1)
            throw std::exception();
        execve(cgiData.cgiArgs[0], cgiData.cgiArgs, cgiData.env);
        releaseExecResources(cgiData);
        perror("error");
        throw std::exception();
    }
    close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
    if (method == "POST")
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
}

int readFromCgi(Webserv &Web, CgiHandler &cgiData, std::string method, size_t &indexClnt)
{

    if (method == "POST")
    {
        if (Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].revents & POLLOUT)
        {
            int bytes_r;
            char buffer[SIZESR];
            bytes_r = read(cgiData.bodyFd, buffer, SIZESR);
            if (bytes_r < 0)
            {
                createCgi(Web, cgiData, method, Web.recs[indexClnt].data, indexClnt);
                return (erorrCgi(cgiData));
            }
            if ((write(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd, buffer, bytes_r)) < 0)
            {
                createCgi(Web, cgiData, method, Web.recs[indexClnt].data, indexClnt);
                return (erorrCgi(cgiData));
            }
        }
    }
    if (Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].revents & (POLLIN | POLLHUP))
    {

        char buffer[SIZESR];
        int bytes = read(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd, buffer, SIZESR - 1);

        if (bytes < 0)
        {
            createCgi(Web, cgiData, method, Web.recs[indexClnt].data, indexClnt);
            return (erorrCgi(cgiData));
        }
        
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

int runCgi(Webserv &Web, Record &log, Request req, size_t &indexClnt)
{
    if (log.status == NEW_RS)
    {
        log.isItCgi = true;//! remove
        if (createPipes(Web, log.cgiData, req, indexClnt))
            return (1);
        log.status = CGI_ACTIVE_RS;
        log.method = CGI_RDWR;
        createCgi(Web, log.cgiData, req.method, log.data, indexClnt);

        releaseExecResources(log.cgiData);
    }
    else
    {
        if (readFromCgi(Web, log.cgiData, req.method, indexClnt))
        {
            closeCGI(Web, log.cgiData, req.method, indexClnt);
            return (1);
        }
        if (log.cgiData.canRead) // *CHEEEECK 
        {
            parseCgiResponse(log.cgiData, req);
            log.cgiData.canRead = false;
        }
    }
    return 0;
}

void getCgiUtils(playloadData &data, Request req, CgiHandler &cgiData, std::vector<int> &listenPorts) // put them on the class so you'll have refer
{
    std::string pathInfo = "PATH_INFO=";
    std::string query = "QUERY_STRING=";
    size_t k = data.cgiInfo.find('?');
    if (k != std::string::npos)
    {
        pathInfo += data.cgiInfo.substr(0, k);
        query += data.cgiInfo.substr(k + 1);
    }
    else
        pathInfo += data.cgiInfo;
    std::stringstream ss;
    ss << listenPorts[0];
    std::string port = ss.str();
    // headers.push_back("AUTH") //!
    cgiData.headers.push_back("GATEWAY_INTERFACE=CGI/1.1");
    cgiData.headers.push_back(pathInfo);
    cgiData.headers.push_back(query);
    cgiData.headers.push_back("REQUEST_METHOD=" + req.method);
    cgiData.headers.push_back("REDIRECT_STATUS=200");
    cgiData.headers.push_back("SCRIPT_NAME=" + data.cgiFile);
    cgiData.headers.push_back("SCRIPT_FILENAME=" + data.cgiScriptFile);
    cgiData.headers.push_back("SERVER_PORT=" + port); // the right server port?
    cgiData.headers.push_back("SERVER_PROTOCOL=HTTP/1.0");

    std::map<std::string, std::string>::iterator iter;
    for (iter = req.headers.begin(); iter != req.headers.end(); iter++)
    {
        if (iter->first == "content-length")
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
        if (!cgiData.cgStatusName.empty() && data.table[cgiData.cgStatusCode] != cgiData.cgStatusName)
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
    //if no body
    if (cgiData.cgBodyFileName.empty())
    {
        log.status = CLOSED_RS;
        return false;
    }
    log.fd = open(cgiData.cgBodyFileName.c_str(), O_RDONLY);
    struct stat info;
    if (log.fd == -1 || stat(cgiData.cgBodyFileName.c_str(), &info) == -1)
    {
        data.statusCode = 500;
        return 1;
    }
    data.bodySize = info.st_size;
    log.status = CLOSED_RS;
    if (getFile(log))
        return 1;
    return 0;
}

int cgiProcess(Webserv &Web, Record &log, Request req, size_t &i)
{
    if (runCgi(Web, log, req, i))
    {
        log.status = ERROR_RS;
    }
    if (log.cgiData.dataReady == true || log.status == ERROR_RS)
    {
        if (log.cgiData.fd != -1)
        {
            close(log.cgiData.fd), log.cgiData.fd = -1;
        }
        if (log.cgiData.dataReady)
        {
            closeCGI(Web, log.cgiData, req.method, i);
        }
        if (cgiResponseHandler(log, log.cgiData, log.data))
            return 1;
        else
            log.cgiData.cgiValid = true;
    }
    return 0;
}

int cgiHandler(Webserv &Web, Record &log, Request req, std::vector<int> &listenPorts, size_t &i)
{
    // std::vector<char *> exFile;
    // std::vector<char *> env;
    // std::vector<std::string> headers;
    if (log.status == NEW_RS)
        log.cgiData.dataReady = false;
    getCgiUtils(log.data, req, log.cgiData, listenPorts);
    if (cgiProcess(Web, log, req, i))
        return 1;
    return 0;
}
