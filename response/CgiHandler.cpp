#include "CgiHandler.hpp"

int erorrCgi(CgiHandler &cgiData)
{
    cgiData.cgStatusCode = 500;
    cgiData.cgStatusName = "Internal Server Error";
    return 1;
}
CgiHandler::CgiHandler()
{
    dataReady = false;
    cgiValid = false; 
    canRead = false;
}

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

int parseCgiResponse(CgiHandler &cgiData)
{
    //conditions : must be key: value, and content type, otherWise 502 must be set to : cgiData.cgStatusCode
    //and "Bad Gateway" to cgiData.cgStatusName and return 
        //                process
        //                   |
        //                   |
        //                   |
        //                   |
    //result: cgiHeadersData + bodyFile
    // case error happen while creating BodyFile , 500 should be set to cgiData.cgStatusCode
    // "Internal Server Error" in cgiData.cgStatusName and return 1
    cgiData.cgBuff.clear();
    return 0;
}

int NewCgi(Record &log, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    (void)log;
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (erorrCgi(cgiData));

    for (int i = 0; i < 2; i++)
    {
        pollfd client_poll;
        client_poll.fd = outputPipe[i];
        client_poll.revents = 0;
        obj.fds.push_back(client_poll);
        obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
        obj.recs.push_back(Record());
        obj.req.push_back(Request());
        obj.data.push_back(Data());
    }
    obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].events = POLLIN;
    obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].events = POLLOUT;
    if (method == "POST")
    {
        int inputPipe[2];
        cgiData.bodyFd = open(bodyFile.c_str(), O_RDONLY, 0644);
        if (cgiData.bodyFd < 0)
            return (erorrCgi(cgiData));
        if (pipe(inputPipe) == -1)
            return (erorrCgi(cgiData));
        for (int i = 0; i < 2; i++)
        {
            pollfd client_poll;
            client_poll.fd = inputPipe[i];
            client_poll.revents = 0;
            obj.fds.push_back(client_poll);
            obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
            obj.recs.push_back(Record());
            obj.req.push_back(Request());
            obj.data.push_back(Data());
        }
        obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].events = POLLIN;
        obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].events = POLLOUT;
    }
    return (0);
}


int outputcgi(Server &obj, CgiHandler &cgiData, std::string method)
{
    // if (1)
    if (obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].revents & POLLOUT)
    {
        if (method == "POST")
        {
            if (dup2(STDIN_FILENO, obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].fd) < 0)
                return (erorrCgi(cgiData));
            close(obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].fd);
            close(obj.fds[obj.recs[obj.iClnt].idFdPipe[3]].fd);
        }

        if (dup2(obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].fd, STDOUT_FILENO) < 0)
            return (erorrCgi(cgiData));
        close(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd);
        close(obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].fd);

        execve(cgiData.path.c_str(), cgiData.executeablePath, cgiData.env);
    }
    return (0);
}

int readFromCgi(Server &obj, CgiHandler &cgiData, std::string method)
{
    close(obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].fd);

    if (method == "POST")
    {
        close(obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].fd);
        if (obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].revents & POLLOUT)
        {
            int bytes_r;
            char buffer[SIZESR];
            
            bytes_r = read(cgiData.bodyFd, buffer, SIZESR);
            if (bytes_r < 0)
                return (erorrCgi(cgiData));

            if (write(obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].fd, buffer, SIZESR) < 0)
                return (erorrCgi(cgiData));
        }
    }

    // if (obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].revents & POLLIN)
    if (1)
    {
        char buffer[SIZESR];
        int bytes = read(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd, buffer, SIZESR-1);
        if (bytes < 0)
            return (erorrCgi(cgiData));
        cgiData.cgBuff = *buffer;
        buffer[bytes] = '\0';
        std::cout << "buffer =   " << buffer << " emd "<< std::endl;
    }

    return (0);
}



int runCgi(Record &log, Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    if (log.status == NEW_RS)
    {
        // std::cout << log.status;
        std::cout << "status: " << log.status << std::endl;
        if (NewCgi(log, obj, cgiData, method, bodyFile))
            return(1);
        std::cout << "status: " << log.status << std::endl;
        // std::cout << "NEW_RSS\n";
    }
    else
    {
        log.status = CGI_ACTIVE;
        log.method = CGI_RDWR;
        pid_t pid = fork();
        if (pid == -1)
            return (erorrCgi(cgiData));
        
        if (pid == 0)
        {
            // std::cout << "cild prosses \n\n";
            if (outputcgi(obj, cgiData,method))
                return (1);
            // std::cout << "finsh   cild\n\n";
        }
        else
        {
            // std::cout << "parante prosses \n\n";
            if (readFromCgi(obj, cgiData,method))
                return (1);
        }
        // std::cout << "finsh   parnt\n\n";
        // if (cgiData.canRead) /* CHEEEECK /!\*/
        //     parseCgiResponse(cgiData);
    }
    return 0;
}

void getCgiEnv(playloadData &data, Request &req, CgiHandler &cgiData, std::vector<int> &listenPorts)
{
    std::string pathInfo = "PATH_INFO=";
    std::string query = "QUERY_STRING=";
    std::vector<std::string> headers;
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
    std::vector<char *> env;
    std::vector<char *> exFile;
    for (size_t i = 0; i < headers.size(); i++)
        env.push_back((char *)headers[i].c_str());
    env.push_back(NULL);
    cgiData.env = &env[0];

    exFile.push_back((char *)data.cgiExecutePath.c_str());
    exFile.push_back(NULL);
    cgiData.executeablePath = &exFile[0];
    cgiData.path = data.cgiScriptFile;

// int i = -1;
// while (cgiData.env[++i])
//     std::cout << env[i] << std::endl;

}

int cgiResponseHandler(Record &log, CgiHandler &cgiData, playloadData &data)
{
    if (cgiData.cgStatusCode != 200)
    {
        data.statusName = cgiData.cgStatusName;
        data.statusCode = cgiData.cgStatusCode;
        return 1;
    }
    log.fd = open(cgiData.cgBodyFileName.c_str(), O_RDONLY);
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

int cgiProcess(CgiHandler &cgiData, Record &log, Server &obj, playloadData &data, Request &req)
{
    // he should set log.status to CGI_ACTIVE and log.method to CGI_RDWR
    // he should set cgiData.cgStatusName and cgiData.cgStatusCode
        // to its proper data
    // he should check whether the parser ends or not to close the file was opened
    // std::cout << "true " << log.status << "\n\n";
    if (runCgi(log, obj, cgiData, req.method, req.bodyPath))
        log.status = ERROR_RS;
    // std::cout << "true " << log.status << "\n\n";
    
    if (cgiData.dataReady == true || log.status == ERROR_RS)
    {
        if (cgiResponseHandler(log, cgiData, data))
            return 1;
        else
            cgiData.cgiValid = true;
    }
    return 0;
}

int cgiHandler(Record &log, CgiHandler &cgiData, Server &obj, playloadData &data, Request &req, std::vector<int> &listenPorts)
{
    if (log.status == NEW_RS)
        cgiData.dataReady = false;
    getCgiEnv(data, req, cgiData, listenPorts);

    if (cgiProcess(cgiData, log, obj, data, req))
        return 1;
    return 0;
}
