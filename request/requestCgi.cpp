#include "../response/CgiHandler.hpp"
#include "../config_file/config_file.hpp"
#include "../server/server.hpp"

std::string tolowerStri(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}

bool isHoldSpaceCgi(const std::string& str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (std::isspace(str.at(i)))
            return (false);
    }
    return (true);
}

std::string getRandomName(CgiHandler &cgi)
{
    std::stringstream ss;
    time_t tStamp = time(NULL);
    struct tm *currentTime = localtime(&tStamp);
    char arr[9];

    strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    ss << cgi.count++;
    randName += '_';
    randName += ss.str();
    return randName;
}

std::string trimCgi(const std::string& str)
{
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start]))
        start++;

    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1]))
        end--;

    return str.substr(start, end - start);
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


bool isLfCrCgi(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (isspace(static_cast<unsigned char>(str[i])))
            return false;
    }
    return (true);
}

bool    parseLine(std::string str, CgiHandler &cgiData)
{
    std::stringstream iss(str);
    iss >> cgiData.cgStatusCode >> cgiData.cgStatusName;
    if (!cgiData.cgStatusName.empty() && cgiData.cgStatusName.at(0) == ' ')
        cgiData.cgStatusName.erase(0, 1);
    if (cgiData.cgStatusCode >= 200 && cgiData.cgStatusCode <= 599)
        return (true);
    else
        return (false);
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
        if (cgiData.cgHeaders.find("content-type") == cgiData.cgHeaders.end())
        {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return;
        }
        if (cgiData.cgHeaders.find("status") != cgiData.cgHeaders.end())
        {
            if (parseLine(cgiData.cgHeaders["status"], cgiData) == false)
            {
                cgiData.cgStatusCode = 502;
                cgiData.cgStatusName = "Bad Gateway";
                cgiData.finFlag = true;
                return;
            }

        }
        return;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (!isPrintableC(key) || !isHoldSpaceCgi(key))
        {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return ;
        }
        value = trimCgi(value);
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
    if (cgiData.lenRead >= MAX_LEN_HEADER_FEILD)
    {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return ;
    }
    return;
}

void getBody(CgiHandler &cgiData)
{
    cgiData.lenRead += cgiData.fullLine.length();
    if (cgiData.fd == -1)
    {
        cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("clientCgi_") + getRandomName(cgiData);
        cgiData.bodyLen = 0;
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
        {
            cgiData.cgStatusCode = 500;
            cgiData.cgStatusName = "Internal Server Error";
            return ;
        }
    }
    cgiData.bodyLen += cgiData.fullLine.length();
    if (cgiData.bodyLen > 1024 * 100000) // after handel 
    {
        cgiData.cgStatusCode = 502;
        cgiData.getEndFile = true;
        return ;
    }
    if (cgiData.endRead == false)
    {
        if (cgiData.bodyLen != 0)
        {
            if (write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length()) == -1)
                return (returnedInternalError(cgiData));
            cgiData.fullLine.clear();
        }
        if (cgiData.dataReady == true)
            close(cgiData.fd);
        return ;
    }
}

int parseCgiResponse(CgiHandler &cgiData)
{
    size_t pos;
    std::string tes;
    cgiData.fullLine.append(cgiData.cgBuff.c_str(), cgiData.cgBuff.length());
    while (1)
    {
        pos = cgiData.fullLine.find("\r\n");
        if (pos == std::string::npos && cgiData.getEndHeaders == false)
            break ;
        tes = cgiData.fullLine;
        if (cgiData.getEndHeaders == false)
            getHeadersCgi(cgiData);
        else
            cgiData.reqState = SECOND_CGI_ST;
        if (cgiData.getEndHeaders && cgiData.getEndFile == false)
            getBody(cgiData);
        if (cgiData.lenRead == cgiData.cgBuff.length())
        {
            if ((cgiData.cgStatusCode == 502) || cgiData.getEndFile || (pos != std::string::npos && tes.length() == 2))
                cgiData.clientState = CLIENT_CGI_ON;
            cgiData.lenRead = 0;
            break ;
        }    
        if (cgiData.getEndFile)
            cgiData.finFlag = true;

        if (cgiData.finFlag)
        {
            // cgiData.reqState = SECOND_CGI_ST;
            cgiData.clientState = CLIENT_CGI_ON;
            break;
        }
    }
    cgiData.cgBuff.clear();
    return 0;
}
