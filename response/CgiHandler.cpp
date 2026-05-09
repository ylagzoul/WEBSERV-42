#include "CgiHandler.hpp"

std::string normalizeKey(std::string key)
{
    for(size_t i = 0; i < key.length(); i++)
    {
        key[i] = toupper(key[i]);
        if (key[i] == '-')
            key[i] = '_';
    }
    return key;
}

int runCgi(pollfd &pEvents, CgiHandler &cgiData, std::string method, std::string bodyFile)
{
    (void)cgiData;
    (void)method;
    //if method post
    ////    write to cgi
    (void)bodyFile;
    (void)pEvents;

    return 0;
}

void getCgiEnv(playloadData &data, Request &req, CgiHandler &cgiData)
{
    std::string pathInfo = "PATH_INFO=";
    std::string query = "QUERY_STRING=";
    std::vector<std::string> headers;
    size_t k = data.cgiInfo.find('?');
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
    // headers.push_back("SERVER_PORT=" /*+ port*/);
    headers.push_back("SERVER_PROTOCOL=HTTP/1.0");

    std::map<std::string, std::string>::iterator iter;
    for(iter = req.headers.begin(); iter != req.headers.end(); iter++)
    {
        if (iter->first == "content-lenght")
            headers.push_back("CONTENT_LENGTH=" + iter->second);
        else if (iter->second == "content-type")
            headers.push_back("CONTENT_TYPE=" + iter->second);
        else
            headers.push_back("HTTP_" + normalizeKey(iter->first) + "=" + iter->second);
    }
    //                      check                     //
    std::vector<const char *>env; // to char *
    for(size_t i = 0; i < headers.size(); i++)
        env.push_back(headers[i].c_str());
    env.push_back(NULL);
    cgiData.env = env.data();
    cgiData.executeablePath = data.cgiExecutePath;
    cgiData.path = data.cgiScriptFile;
    
}
// int i = -1;
// while(envp[++i])
//     std::cout << envp[i] << std::endl;


int cgiHandler(pollfd &pEvents, playloadData &data, Request &req)
{
    CgiHandler cgiData;
    getCgiEnv(data, req, cgiData);
    runCgi(pEvents, cgiData, req.method, req.bodyPath);
    return 0;
}
