#include "CgiHandler.hpp"
#include "../server/server.hpp"

void createExecResources(CgiHandler &cgiData, PlayloadData &data)
{
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
        cgiData.env = NULL;
    }
    if (cgiData.cgiArgs)
    {
        delete[] cgiData.cgiArgs[0];
        delete[] cgiData.cgiArgs[1];
        delete[] cgiData.cgiArgs;
        cgiData.cgiArgs = NULL;
    }
}

void closeCGI(Webserv &Web, std::string method, size_t &indexClnt)
{
    if (method == "POST")
    {
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd);
        Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[2]] = "close";
        Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[3]] = "close";
        Web.server[Web.s].serverPort[Web.t].reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[2]);
        Web.server[Web.s].serverPort[Web.t].reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[3]);
    }
    close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
    Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[0]] = "close";
    Web.statusFd[Web.recs[indexClnt].IndexFdsPipe[1]] = "close";
    Web.server[Web.s].serverPort[Web.t].reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[0]);
    Web.server[Web.s].serverPort[Web.t].reuseFd.push_back(Web.recs[indexClnt].IndexFdsPipe[1]);
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

CgiHandler::CgiHandler() : bodyLen(0), fd(-1), cgStatusCode(200), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST), endRead(false), count(0)
{
    fullLine = "";
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

    if (Web.server[Web.s].serverPort[Web.t].reuseFd.size() == 0)
    {
        Web.fds.push_back(client_poll);
        Web.typeFd[Web.fds.size() - 1] = "pipe";
        Web.statusFd[Web.fds.size() - 1] = "active";
        Web.server[Web.s].serverPort[Web.t].index_Client.push_back(Web.fds.size() - 1);
        Web.recs.push_back(Record());
        Web.req.push_back(Request());
        Web.recs[indexClnt].IndexFdsPipe.push_back(Web.fds.size() - 1);
    }
    else
    {
        Web.fds[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = client_poll;
        Web.typeFd[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = "pipe";
        Web.statusFd[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = "active";
        Web.req[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = Request();
        Web.recs[Web.server[Web.s].serverPort[Web.t].reuseFd[0]] = Record();
        Web.recs[indexClnt].IndexFdsPipe.push_back(Web.server[Web.s].serverPort[Web.t].reuseFd[0]);
        Web.server[Web.s].serverPort[Web.t].reuseFd.erase(Web.server[Web.s].serverPort[Web.t].reuseFd.begin() + 0);
    }
    fcntl(fdPipe, F_SETFL, O_NONBLOCK);
}

int createPipes(Webserv &Web, CgiHandler &cgiData, Request req, size_t &indexClnt)
{
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (1);

    addFdPollfd(Web, indexClnt, outputPipe[0], POLLIN);
    addFdPollfd(Web, indexClnt, outputPipe[1], POLLOUT);

    if (req.method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(req.bodyPath.c_str(), O_RDONLY);
        if (cgiData.bodyFd < 0)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            return (1);
        }

        if (pipe(inputPipe) == -1)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            return (1);
        }
        addFdPollfd(Web, indexClnt, inputPipe[0], POLLIN);
        addFdPollfd(Web, indexClnt, inputPipe[1], POLLOUT);
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

int createCgi(Webserv &Web, CgiHandler &cgiData, std::string method, PlayloadData &data, size_t &indexClnt)
{
    cgiData.pid = fork();
    if (cgiData.pid == -1)
    {
        if (method == "POST")
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
        closeCGI(Web, Web.req[indexClnt].method, indexClnt);
        return (1);
    }
    if (cgiData.pid == 0)
    {
        // close(2); //! youssef check if its not a problem
        if (method == "POST")
        {
            if (dup2(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd, STDIN_FILENO) < 0)
            {
                close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
                close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
                closeCGI(Web, Web.req[indexClnt].method, indexClnt);
                throw std::runtime_error("ERROR EXITED");
            }
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[2]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
        }
        if (dup2(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd, STDOUT_FILENO) < 0)
        {
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
            close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
            throw std::runtime_error("ERROR EXITED");
        }
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd);
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);

        if (chdir(switchData(data.realPath).c_str()) == -1)
            throw std::runtime_error("ERROR EXITED");

        execve(cgiData.cgiArgs[0], cgiData.cgiArgs, cgiData.env);
        std::cerr << "asdfdsfds\n";
        releaseExecResources(cgiData);
        throw std::runtime_error("ERROR EXITED");
    }

    close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd);
    if (method == "POST")
        close(Web.fds[Web.recs[indexClnt].IndexFdsPipe[3]].fd);
    return (0);
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
                return (1);
            }
            if ((write(Web.fds[Web.recs[indexClnt].IndexFdsPipe[1]].fd, buffer, bytes_r)) < 0)
                return (1);
        }
    }
    if (Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].revents & (POLLIN | POLLHUP))
    {

        char buffer[SIZESR];
        int bytes = read(Web.fds[Web.recs[indexClnt].IndexFdsPipe[0]].fd, buffer, SIZESR - 1);
        if (bytes < 0)
            return (1);

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
        if (createPipes(Web, log.cgiData, req, indexClnt))
        {
            Web.recs[indexClnt].cgiData.status = 500;
            return (1);
        }
        log.status = CGI_ACTIVE_RS;
        log.method = CGI_RDWR;
        if (createCgi(Web, log.cgiData, req.method, log.data, indexClnt))
        {
            Web.recs[indexClnt].cgiData.status = 500;
            return (1);
        }
        releaseExecResources(log.cgiData);
    }
    else
    {
        int ret;
        int status = 0;
        ret = waitpid(Web.recs[indexClnt].cgiData.pid, &status, WNOHANG);
        // if (ret == -1)
        //     Web.recs[indexClnt].cgiData.status = 500;
        if (ret > 0)
        {
            // if (WIFSIGNALED(status))
            // {
            //     Web.recs[indexClnt].cgiData.status = 500;
            // }
            if (WIFEXITED(status))
            {
                if (WEXITSTATUS(status) != 0)
                    Web.recs[indexClnt].cgiData.status = 500;
            }
        }
        status = 0;
        if (time(NULL) - Web.recs[indexClnt].time > 60)
        {
            kill(Web.recs[indexClnt].cgiData.pid, SIGKILL);
            ret = waitpid(Web.recs[indexClnt].cgiData.pid, &status, 0);
            if (ret > 0)
                Web.recs[indexClnt].cgiData.status = 504;
        }
        if (log.cgiData.status == 504 || log.cgiData.status == 500)
            return (1);

        if (readFromCgi(Web, log.cgiData, req.method, indexClnt))
        {
            Web.recs[indexClnt].cgiData.status = 500;
            return (1);
        }
        if (log.cgiData.canRead || log.cgiData.endRead)
        {
            parseCgiResponse(log.cgiData, req);
            log.cgiData.canRead = false;
        }
    }

    return 0;
}

void getCgiUtils(PlayloadData &data, Request req, CgiHandler &cgiData, std::vector<int> &listenPorts)
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
    cgiData.headers.push_back("GATEWAY_INTERFACE=CGI/1.1");
    cgiData.headers.push_back(pathInfo);
    cgiData.headers.push_back(query);
    cgiData.headers.push_back("REQUEST_METHOD=" + req.method);
    cgiData.headers.push_back("REDIRECT_STATUS=200");
    cgiData.headers.push_back("SCRIPT_NAME=" + data.cgiFile);
    cgiData.headers.push_back("SCRIPT_FILENAME=" + data.cgiScriptFile);
    cgiData.headers.push_back("SERVER_PORT=" + port);
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
}

bool cgiResponseHandler(Record &log, CgiHandler &cgiData, PlayloadData &data, Request req)
{
    if (req.method == "POST")
    {
        if (access(req.bodyPath.c_str(), F_OK) != -1)
            std::remove(req.bodyPath.c_str());
    }
    if (cgiData.status == 500 || cgiData.status == 502 || cgiData.status == 504)
    {
        data.statusCode = cgiData.status;
        return true;
    }
    if (data.table.count(cgiData.cgStatusCode))
    {
        if (!cgiData.cgStatusName.empty() && data.table[cgiData.cgStatusCode] != cgiData.cgStatusName)
        {
            data.statusCode = 502;
            return true;
        }
        data.statusCode = cgiData.cgStatusCode;
    }
    else
    {
        data.statusCode = 502;
        return true;
    }
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
        return true;
    }
    data.bodySize = info.st_size;
    log.status = CLOSED_RS;
    if (getFile(log))
        return true;
    return false;
}

bool cgiProcess(Webserv &Web, Record &log, Request req, size_t &i)
{
    if (runCgi(Web, log, req, i))
    {
        closeCGI(Web, req.method, i);
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
            closeCGI(Web, req.method, i);
        }
        if (cgiResponseHandler(log, log.cgiData, log.data, req))
        {
            if (access(log.cgiData.cgBodyFileName.c_str(), F_OK) != -1)
                remove(log.cgiData.cgBodyFileName.c_str());
            return true;
        }
        else
            log.cgiData.cgiValid = true;
    }
    return false;
}

bool cgiHandler(Webserv &Web, Record &log, Request req, std::vector<int> &listenPorts, size_t &i)
{
    if (log.status == NEW_RS)
        log.cgiData.dataReady = false;
    getCgiUtils(log.data, req, log.cgiData, listenPorts);
    if (cgiProcess(Web, log, req, i))
        return true;
    return false;
}

