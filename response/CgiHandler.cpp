#include "CgiHandler.hpp"

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

int runCgi(Server &obj, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    (void)method;
    (void)cgiData;
    (void)bodyFile;
    (void)obj;

    // if (method == "POST")
    // {
    //     if (1 /*newcgi == true*/)
    //     {
    //         int inFd[2];
    //         int outFd[2];
    //         if (pipe(inFd) == -1 || pipe(outFd) == -1)
    //         {
    //             std::cout << "Pipe Failed" << std::endl;
    //             exit(1);
    //         }
    //         for (int i = 0; i < 2; i++)
    //         {
    //             pollfd client_poll;
    //             client_poll.fd = inFd[i];
    //             client_poll.events = POLLIN;
    //             client_poll.revents = 0;
    //             obj.fds.push_back(client_poll);
    //             obj.recs[obj.iClnt].idFdPipe[i] = obj.fds.size();
    //             obj.recs.push_back(Record());
    //             obj.req.push_back(Request());
    //             obj.data.push_back(Data());
    //         }
    //         for (int i = 0; i < 2; i++)
    //         {
    //             pollfd client_poll;
    //             client_poll.fd = outFd[i];
    //             client_poll.events = POLLIN;
    //             client_poll.revents = 0;
    //             obj.fds.push_back(client_poll);
    //             obj.recs[obj.iClnt].idFdPipe[i + 2] = obj.fds.size() - 1;
    //             obj.recs.push_back(Record());
    //             obj.req.push_back(Request());
    //             obj.data.push_back(Data());
    //         }
    //     }
    // }
    //     pid_t pid = fork();
    //     if (pid == -1)
    //     {
    //         std::cout << "fork Failed" << std::endl;
    //         exit(1);
    //     }
    //     //config_file  errors  files  request  response  server
    //     if (pid == 0)
    //     {
    //         close(obj.fds[obj.recs[obj.iClnt].idFdPipe[0]].fd);

    //         dup2(obj.fds[obj.recs[obj.iClnt].idFdPipe[1]].fd, STDOUT_FILENO);

    //         execve(cgiData.path.c_str(), cgiData.executeablePath, cgiData.env);
    //     }
    //     else
    //     {
    //     // while reading ---> state = pipe_read
    //         close(obj.fds[obj.recs[obj.iClnt].idFdPipe[2]].fd);

    //         // read();
    //     }
    // }
    // else if (method == "GET")
    // {
    //     ;
    // }

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
