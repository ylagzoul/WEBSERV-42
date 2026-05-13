#include "CgiHandler.hpp"

int erorrCgi(CgiHandler &cgiData)
{
    cgiData.cgStatusCode = 500;
    cgiData.cgStatusName = "Internal Server Error";
    return 1;
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

int NewCgi(Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    int outputPipe[2];
    if (pipe(outputPipe) == -1)
        return (erorrCgi(cgiData));

    for (int i = 0; i < 2; i++)
    {
        pollfd client_poll;
        client_poll.fd = outputPipe[i];
        client_poll.events = POLLIN;
        client_poll.revents = 0;
        obj.fds.push_back(client_poll);
        obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
        obj.recs.push_back(Record());
        obj.req.push_back(Request());
        obj.data.push_back(Data());
    }
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
            client_poll.events = POLLIN;
            client_poll.revents = 0;
            obj.fds.push_back(client_poll);
            obj.recs[obj.iClnt].idFdPipe.push_back(obj.fds.size() - 1);
            obj.recs.push_back(Record());
            obj.req.push_back(Request());
            obj.data.push_back(Data());
        }
        obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].events = POLLOUT;
    }
    return (0);
}


int outputcgi(Server &obj, CgiHandler &cgiData, std::string method)
{
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

    if (obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].revents & POLLIN)
    {
        char buffer[SIZESR];
        int bytes = read(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd, buffer, SIZESR-1);
        if (bytes < 0)
            return (erorrCgi(cgiData));
        cgiData.cgBuff = *buffer;
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }
    return (0);
}


int runCgi(Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    if (1 /*newcgi == true*/)
    {
        if (NewCgi(obj, cgiData, method, bodyFile))
            return(1);
    }
    
    pid_t pid = fork();
    if (pid == -1)
        return (erorrCgi(cgiData));
    

    if (pid == 0)
    {
        if (outputcgi(obj, cgiData,method))
            return (1);
    }
    else
    {
        if (readFromCgi(obj, cgiData,method))
            return (1);
        // std::cout << "youssef11\n";
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
    cgiData.env = env.data();

    exFile.push_back((char *)data.cgiExecutePath.c_str());
    exFile.push_back(NULL);
    cgiData.executeablePath = exFile.data();
    cgiData.path = data.cgiScriptFile;
}

// int i = -1;
// while (cgiData.env[++i])
//     std::cout << env[i] << std::endl;

int cgiHandler(Server &obj, playloadData &data, Request &req, std::vector<int> &listenPorts)
{
    CgiHandler cgiData;
    getCgiEnv(data, req, cgiData, listenPorts);
    runCgi(obj, cgiData, req.method, req.bodyPath);
    return 0;
}
