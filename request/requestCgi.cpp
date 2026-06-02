#include "../response/CgiHandler.hpp"

std::string tolowerStri(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}

bool isPrintableC(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!isprint(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

std::string toStrC(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
}


bool isLfCrCgi(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (isspace(static_cast<unsigned char>(str[i])))
            return false;
    }
    return (true);
}

void    parseLine(std::string str, CgiHandler &cgiData)
{
    std::stringstream iss(str);
    iss >> cgiData.cgStatusCode >> cgiData.cgStatusName;
    if (!cgiData.cgStatusName.empty() && cgiData.cgStatusName.at(0) == ' ')
        cgiData.cgStatusName.erase(0, 1);
}

void getHeadersCgi(CgiHandler &cgiData)
{
    size_t endLine = cgiData.fullLine.find("\r\n");
    if (endLine == std::string::npos)
        return;
    std::string line = cgiData.fullLine.substr(0, endLine);
    if (line.empty())
    {
        cgiData.getEndHeaders = true;
        cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
        cgiData.fullLine.erase(0, endLine + 2);
        if (cgiData.cgHeaders.find("content-length") == cgiData.cgHeaders.end() || cgiData.cgHeaders.find("content-type") == cgiData.cgHeaders.end())
        {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return;
        }
        if (cgiData.cgHeaders.find("status") != cgiData.cgHeaders.end())
            parseLine(cgiData.cgHeaders["status"], cgiData);
        cgiData.contentLength = std::atoi(cgiData.cgHeaders["content-length"].c_str());
        return;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
        value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
        if (!isPrintableC(key) || !isLfCrCgi(value))
        {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return ;
        }
        cgiData.cgHeaders[tolowerStri(key)] = value;
    }
    else
    {
        cgiData.cgStatusCode = 502;
        cgiData.cgStatusName = "Bad Gateway";
        cgiData.finFlag = true;
        return ;
    }
    cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
    cgiData.fullLine.erase(0, endLine + 2);
    return;
}


void getBody(CgiHandler &cgiData)
{
    static size_t bodyLen;
    cgiData.lenRead += cgiData.fullLine.length();
    if (cgiData.fd == -1)
    {
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
        {
            std::cerr << "Error opening file\n";
            cgiData.cgStatusCode = 500;
            cgiData.cgStatusName = "Internal Server Error";
            return ;
        }
    }
    bodyLen += cgiData.fullLine.length();
    if (bodyLen > 10000000) // after handel
    {
        cgiData.cgStatusCode = 400;
        cgiData.getEndFile = true;
        return;
    }
    if (bodyLen < cgiData.contentLength)
    {
        if (bodyLen != 0)
        {
            write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length());
            cgiData.fullLine.clear();
        }
        return;
    }
    int sizePa = (cgiData.contentLength + cgiData.fullLine.length()) - bodyLen;
    std::string d = cgiData.fullLine.substr(0, sizePa);

    write(cgiData.fd, d.c_str(), sizePa);
    close(cgiData.fd);
    cgiData.getEndFile = true;
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

    size_t pos;
    std::string tes;
    cgiData.fullLine.append(cgiData.cgBuff.c_str(), cgiData.cgBuff.length());
    cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("cgiCleint") + toStrC(1);
    // cout << cgiData.cgBuff.length() << std::endl;
    while (1)
    {
        pos = cgiData.fullLine.find("\r\n");
        if (pos == std::string::npos)
            break ;
        tes = cgiData.fullLine;
        if (cgiData.getEndHeaders == false)
            getHeadersCgi(cgiData);
        else
            cgiData.reqState = SECOND_CGI_ST;
        if (cgiData.getEndHeaders && cgiData.getEndFile == false && cgiData.contentLength > 0)
            getBody(cgiData);
        if (cgiData.lenRead == cgiData.cgBuff.length())
        {
            cout << cgiData.getEndFile << std::endl;
            cout << tes.length() << std::endl;

            if ((cgiData.cgStatusCode == 502) || cgiData.getEndFile || (pos != std::string::npos && tes.length() == 2))
                cgiData.clientState = CLIENT_CGI_ON;
            cgiData.lenRead = 0;    
            break ;
        }    
        // // else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        // //     getChunkedBody(req);
        // if (cgiData.getEndHeaders)
        //     cgiData.finFlag = true;
        if (cgiData.getEndFile)
            cgiData.finFlag = true;

        if (cgiData.finFlag)
        {
            cgiData.reqState = SECOND_CGI_ST;
            cgiData.clientState = CLIENT_CGI_ON;
            break;
        }
    }

    cgiData.cgBuff.clear();
    return 0;
}
