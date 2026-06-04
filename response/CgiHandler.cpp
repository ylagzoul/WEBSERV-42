#include "CgiHandler.hpp"

int erorrCgi(CgiHandler &cgiData)
{
    cgiData.cgStatusCode = 500;
    cgiData.cgStatusName = "Internal Server Error";
    return 1;
}
CgiHandler::CgiHandler() : fd(-1), cgStatusCode(200), cgStatusName("OK"), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST)
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

int parseCgiResponse_yl(Server &obj, std::string method, CgiHandler &cgiData)
{

    if (obj.recs[obj.iClnt].cont != 0)
    {
        if (method == "POST")
        {
            close(obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].fd);

            obj.fds.erase(obj.fds.begin() + obj.recs[obj.iClnt].idFdPipe[3]);

            obj.req[obj.recs[obj.iClnt].idFdPipe[3]].clean();
            obj.req.erase(obj.req.begin() + obj.recs[obj.iClnt].idFdPipe[3]); // iteration problem

            obj.recs[obj.recs[obj.iClnt].idFdPipe[3]].clean();
            obj.recs.erase(obj.recs.begin() + obj.recs[obj.iClnt].idFdPipe[3]);

            obj.data.erase(obj.data.begin() + obj.recs[obj.iClnt].idFdPipe[3]);
        }
        close(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd);

        obj.fds.erase(obj.fds.begin() + obj.recs[obj.iClnt].idFdPipe[0]);
        obj.req[obj.recs[obj.iClnt].idFdPipe[0]].clean();
        obj.req.erase(obj.req.begin() + obj.recs[obj.iClnt].idFdPipe[0]); // iteration problem

        obj.recs[obj.recs[obj.iClnt].idFdPipe[0]].clean();
        obj.recs.erase(obj.recs.begin() + obj.recs[obj.iClnt].idFdPipe[0]);

        obj.data.erase(obj.data.begin() + obj.recs[obj.iClnt].idFdPipe[0]);
        cgiData.dataReady = true;
    }

    return 0;
}

int NewCgi(Record &log, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    (void)log;
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (erorrCgi(cgiData));

    pollfd client_poll;
    client_poll.fd = outputPipe[0];
    client_poll.events = POLLIN;
    client_poll.revents = 0;
    obj.fds.push_back(client_poll);
    obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
    obj.recs.push_back(Record());
    obj.req.push_back(Request());
    obj.data.push_back(Data());

    obj.recs[obj.iClnt].idFdPipe.push_back(outputPipe[1]);

    if (method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(bodyFile.c_str(), O_RDONLY, 0644);
        if (cgiData.bodyFd < 0)
            return (erorrCgi(cgiData));
        if (pipe(inputPipe) == -1)
            return (erorrCgi(cgiData));

        obj.recs[obj.iClnt].idFdPipe.push_back(inputPipe[0]);

        pollfd client_poll;
        client_poll.fd = inputPipe[1];
        client_poll.events = POLLOUT;
        client_poll.revents = 0;
        obj.fds.push_back(client_poll);
        obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
        obj.recs.push_back(Record());
        obj.req.push_back(Request());
        obj.data.push_back(Data());
    }
    return (0);
}

int outputcgi(Server &obj, CgiHandler &cgiData, std::string method)
{
    pid_t pid = fork();
    if (pid == -1)
        return (erorrCgi(cgiData));
    if (pid == 0)
    {
        // std::cout << "llloollllllllllllllllllllllllllllllllllllllll\n";
        if (method == "POST")
        {
            if (dup2(obj.recs[obj.iClnt].idFdPipe[2], STDIN_FILENO) < 0)
                return (erorrCgi(cgiData));
            close(obj.recs[obj.iClnt].idFdPipe[2]);
            close(obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].fd);
        }

        if (dup2(obj.recs[obj.iClnt].idFdPipe[1], STDOUT_FILENO) < 0)
            return (erorrCgi(cgiData));
        close(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd);
        close(obj.recs[obj.iClnt].idFdPipe[1]);

        execve(cgiData.path.c_str(), cgiData.executeablePath, cgiData.env);
        perror("execve");
        exit(1);
    }
    return (0);
}

int readFromCgi(Server &obj, CgiHandler &cgiData, std::string method)
{

    if (method == "POST")
    {
        std::cout << "post read form cgi\n\n";
        if (obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].revents & POLLOUT)
        {
            std::cout << "post writed form cgi if\n\n";
            int bytes_r;
            char buffer[SIZESR];
            bytes_r = read(cgiData.bodyFd, buffer, SIZESR);
            if (bytes_r < 0)
                return (erorrCgi(cgiData));

            if (write(obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].fd, buffer, bytes_r) < 0)
                return (erorrCgi(cgiData));
        }
    }
    if (obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].revents & POLLIN)
    {
        std::cout << " read form cgi\n\n";

        char buffer[SIZESR];
        int bytes = read(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd, buffer, SIZESR - 1);

        std::cout << "buffer = " << buffer << std::endl;
        if (bytes < 0)
            return (erorrCgi(cgiData));

        if (bytes > 0)
        {
            std::cout << "read form cgi -> bytes = " << bytes << std::endl;
            cgiData.canRead = true;
            buffer[bytes] = '\0';
            cgiData.cgBuff.append(buffer, bytes);
            std::cout << "cgiData.cgBuff = " << cgiData.cgBuff << std::endl;
        }
        obj.recs[obj.iClnt].cont++;
    }

    return (0);
}

int runCgi(Record &log, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    if (log.status == NEW_RS)
    {
        std::cout << "new cgi                       .....\n\n";

        if (NewCgi(log, obj, cgiData, method, bodyFile))
            return (1);
        log.status = CGI_ACTIVE;
        log.method = CGI_RDWR;
        if (outputcgi(obj, cgiData, method))
            return (1);
    }
    else
    {
        std::cout << "anadkholch\n\n\n";
        if (readFromCgi(obj, cgiData, method))
            return (1);
        if (cgiData.canRead) /* CHEEEECK /!\*/
        {
            std::cout << "parseCgiResponse   //  cgiData.cgBuff = " << cgiData.cgBuff << std::endl;
            std::cout << "parseCgiResponse\n\n";
            parseCgiResponse(cgiData);
            std::cout << "parseCgiResponse_yl\n\n";
            parseCgiResponse_yl(obj, method, cgiData);
            std::cout << "finsh\n\n";
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

int cgiProcess(CgiHandler &cgiData, Record &log, Server &obj, playloadData &data, Request &req)
{
    // he should set log.status to CGI_ACTIVE and log.method to CGI_RDWR
    // he should set cgiData.cgStatusName and cgiData.cgStatusCode
    // to its proper data
    // he should check whether the parser ends or not to close the file was opened
    // std::cout << "true " << log.status << "\n\n";
    if (runCgi(log, obj, cgiData, req.method, req.bodyPath))
    {
        std::cout << "ERROR_RS\n";
        log.status = ERROR_RS;
    }
    // std::cout << "true " << log.status << "\n\n";

    if (cgiData.dataReady == true || log.status == ERROR_RS)
    {
        if (cgiResponseHandler(log, cgiData, data))
            return 1;
        else
            cgiData.cgiValid = true;
    }
    // std::cout << "true11 " << log.status << "\n\n";
    return 0;
}

int cgiHandler(Record &log, CgiHandler &cgiData, Server &obj, playloadData &data, Request &req, std::vector<int> &listenPorts)
{
    std::vector<char *> exFile;
    std::vector<char *> env;
    std::vector<std::string> headers;
    if (log.status == NEW_RS)
        cgiData.dataReady = false;
    getCgiEnv(headers, env, exFile, data, req, cgiData, listenPorts);
    if (cgiProcess(cgiData, log, obj, data, req))
        return 1;
    // std::cout << "true 9999" << log.status << "\n\n";

    return 0;
}
