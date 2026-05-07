#include "CgiHandler.hpp"

void getCgiEnv(playloadData &data, Request &req, CgiHandler &cgiData)
{
    (void)req;
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
    cgiData.headers.push_back("GATEWAY_INTERFACE=CGI/1.1");
    cgiData.headers.push_back(pathInfo);
    cgiData.headers.push_back(query);
    cgiData.headers.push_back("REQUEST_METHOD=" + req.method);
    cgiData.headers.push_back("REDIRECT_STATUS=200");
    // cgiData.headers.push_back("SCRIPT_NAME=" + data.uri);//path
    // cgiData.headers.push_back("SERVER_PORT=" /*+ port*/);
    cgiData.headers.push_back("SERVER_PROTOCOL=HTTP/1.0");


    std::map<std::string, std::string>::iterator iter;
    for(iter = req.headers.begin(); iter != req.headers.end(); iter++)
    {
        if (iter->first == "content-lenght")
            cgiData.headers.push_back("CONTENT_LENGTH=" + iter->second);
        else if (iter->first == "content-type")
            cgiData.headers.push_back("CONTENT_TYPE=" + iter->second);
        else if (iter->first == "cookie")
            cgiData.headers.push_back("HTTP_COOKIE=" + iter->second);
    }
    
    // for(size_t i = 0; i < cgiData.headers.size(); i++)
    //     std::cout << cgiData.headers[i] << "\n\n";
}


int cgiHandler(playloadData &data, Request &req)
{
    CgiHandler cgiData;
    getCgiEnv(data, req, cgiData);
    return 0;
}