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

int cgiResponseHandler(Record &log, CgiHandler &cgiData, playloadData &data)
{
    if (cgiData.cgStatusCode == 502)
    {
        cgiData.cgStatusName = "Bad Gateway";
        return 1;
    }
    log.fd = open(cgiData.cgBodyFileName.c_str(), O_RDONLY);
    if (log.fd == -1)
    {
        data.status_code = "500";
        data.statusName = "Internal Server Error";
        return 1;
    }
        getFile(log, data);

    return 0;
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

int cgiHandler(CgiHandler &cgiData, Record &log, pollfd &pEvents, playloadData &data, Request &req)
{
    (void)cgiData;
    (void)log;
    (void)pEvents;
    (void)data;
    (void)req;
    data.cgiValid = true;
    // getCgiEnv(data, req, cgiData);
    // if (runCgi(pEvents, cgiData, req.method, req.bodyPath))
    //     return 1;
    // if (cgiData.canRead)
    // {
    //     if (parseCgiResponse(cgiData))
    //         return 1;
    // }
    // if (log.method == CGI_ACTIVE)
    //     return 0;
    // if (cgiResponseHandler(log, cgiData, data))
    //     return 1;
    return 0;
}
