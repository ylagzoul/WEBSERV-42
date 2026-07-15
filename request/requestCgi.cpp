#include "../response/CgiHandler.hpp"
#include "../config_file/config_file.hpp"
#include "../server/server.hpp"

std::string tolowerStri(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}

bool isHoldSpaceCgi(const std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (std::isspace(str.at(i)))
            return (false);
    }
    return (true);
}

std::string trimCgi(const std::string &str)
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

bool isLfCrCgi(const std::string str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str.at(i) == '\r' || str.at(i) == '\n')
            return false;
    }
    return (true);
}

bool isValidStatus(std::string str)
{
    int count = 0;
    for (size_t i = 0; i < 3; i++)
    {
        if (isdigit(str.at(i)))
            count++;
    }
    if (count != 3)
        return (false);
    else
        return (true);
}

bool parseLine(std::string str, CgiHandler &cgiData)
{
    char *tmp;
    if (!isValidStatus(str))
        return (false);
    int st = std::strtol(str.c_str(), &tmp, 10);
    cgiData.cgStatusCode = st;
    cgiData.cgStatusName = trim(tmp);

    if (cgiData.cgStatusCode >= 200 && cgiData.cgStatusCode <= 599)
        return (true);
    else
        return (false);
}

bool getHeadersCgi(CgiHandler &cgiData)
{
    size_t endLine = cgiData.fullLine.find("\r\n");
    if (endLine == std::string::npos)
        return (false);
    std::string line = cgiData.fullLine.substr(0, endLine);
    if (line.empty())
    {
        cgiData.getEndHeaders = true;
        cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
        cgiData.fullLine.erase(0, endLine + 2);
        std::map<std::string, std::string>::iterator it;
        it = cgiData.cgHeaders.find("transfer-encoding");
        std::map<std::string, std::string>::iterator it2;
        if (cgiData.cgHeaders.find("content-type") == cgiData.cgHeaders.end())
            return (returnedBadError(cgiData), true);
        if (it != cgiData.cgHeaders.end() && it->second == "chunked")
        {
            cgiData.contentLength = 0;
            cgiData.bodyType = CHUNKED;
        }
        else
            cgiData.bodyType = FIXED;
        if (cgiData.cgHeaders.find("status") != cgiData.cgHeaders.end())
        {
            if (parseLine(cgiData.cgHeaders["status"], cgiData) == false)
                return (returnedBadError(cgiData), true);
        }
        return (true);
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (!isPrintableC(key) || !isHoldSpaceCgi(key) || key.empty() || !isLfCrCgi(key) || !isLfCrCgi(value))
            return (returnedBadError(cgiData), true);
        value = trimCgi(value);
        if (cgiData.cgHeaders.find(key) != cgiData.cgHeaders.end() && key == "content-length")
            return (returnedBadError(cgiData), true);
        cgiData.cgHeaders[tolowerStri(key)] = value;
        if (cgiData.cgHeaders.find("content-length") != cgiData.cgHeaders.end() && !checkDigits(cgiData, cgiData.cgHeaders["content-length"]))
            return (returnedBadError(cgiData), true);
    }
    else
        return (returnedBadError(cgiData), true);
    cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
    cgiData.countLenHead += cgiData.fullLine.substr(0, endLine + 2).length();
    cgiData.fullLine.erase(0, endLine + 2);
    if (cgiData.countLenHead >= MAX_LEN_HEADER_FEILD)
        return (returnedBadError(cgiData), true);
    return (true);
}

void getBody(CgiHandler &cgiData)
{
    bool isFound = cgiData.cgHeaders.find("content-length") != cgiData.cgHeaders.end();

    cgiData.lenRead += cgiData.fullLine.length();
    if (cgiData.fullLine.length() <= 0)
    {
        cgiData.getEndFile = true;
        return ;
    }
    if (cgiData.fd == -1)
    {
        cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("clientCgi_") + getRandName();
        cgiData.bodyLen = 0;
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
            return (returnedInternalError(cgiData));
    }
    cgiData.bodyLen += cgiData.fullLine.length();
    if (isFound && cgiData.bodyLen < cgiData.contentLength)
    {
        if (write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length()) == -1)
            return (returnedInternalError(cgiData));
        else
        {
            cgiData.fullLine.clear();
            return;
        }
    }
    if (isFound)
    {
        int sizePa = (cgiData.contentLength + cgiData.fullLine.length()) - cgiData.bodyLen;
        std::string d = cgiData.fullLine.substr(0, sizePa);
        if (write(cgiData.fd, d.c_str(), sizePa) == -1)
            return (returnedInternalError(cgiData));
        else
            cgiData.getEndFile = true;
    }
    if (isFound == false)
    {
        if (write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length()) == -1)
            return (returnedInternalError(cgiData));
        else
            cgiData.fullLine.clear();
    }
}

bool getChunked(CgiHandler &cgiData)
{
    size_t endTxt;
    size_t endNum;

    if (cgiData.fd == -1)
    {
        cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("clientCgi_") + getRandName();
        cgiData.bodyLen = 0;
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
            return (returnedInternalError(cgiData), true);
    }
    endNum = cgiData.fullLine.find("\r\n");
    if (endNum != std::string::npos)
    {
        std::string tmp = cgiData.fullLine.substr(0, endNum + 2);
        if (!checkDigits(cgiData, tmp))
            return (returnedBadError(cgiData), true);
        endTxt = cgiData.fullLine.find("\r\n", endNum + 2);
    }
    while (endNum != std::string::npos && endTxt != std::string::npos)
    {
        std::string chunkNum = cgiData.fullLine.substr(0, endNum);
        std::string chunkData = cgiData.fullLine.substr(endNum + 2, endTxt - endNum - 2);
        cgiData.lenRead += cgiData.fullLine.substr(0, endTxt + 2).length();
        cgiData.bodyLen += cgiData.chunkSize;

        if (cgiData.chunkSize == 0 && chunkData.empty())
        {
            if (write(cgiData.fd, chunkData.c_str(), cgiData.chunkSize) == -1)
                return (returnedInternalError(cgiData), true);
            else
            {
                cgiData.fullLine.erase(0, endTxt + 2);
                cgiData.getEndFile = true;
                return (true);
            }
        }
        if (chunkData.length() < cgiData.chunkSize)
            return (returnedBadError(cgiData), true);
        if (write(cgiData.fd, chunkData.c_str(), cgiData.chunkSize) == -1)
            return (returnedInternalError(cgiData), true);
        else
        {
            cgiData.fullLine.erase(0, endTxt + 2);
            endNum = cgiData.fullLine.find("\r\n");
        }
        if (endNum != std::string::npos)
        {
            std::string tmp = cgiData.fullLine.substr(0, endNum + 2);
            if (!checkDigits(cgiData, tmp))
                return (returnedBadError(cgiData), true);
            endTxt = cgiData.fullLine.find("\r\n", endNum + 2);
        }
    }
    return (false);
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
        {
            if (cgiData.endRead == true)
            {
                cgiData.dataReady = true;
                cgiData.status = 502;
            }
            break;
        }
        tes = cgiData.fullLine;
        if (cgiData.getEndHeaders == false)
        {
            if (getHeadersCgi(cgiData) == false)
                break;
            if (cgiData.status == 500 || cgiData.status == 502)
                cgiData.clientState = CLIENT_CGI_ON;
        }
        if (cgiData.getEndHeaders && cgiData.getEndFile == false && cgiData.bodyType == CHUNKED)
        {
            if (getChunked(cgiData) == false)
            {
                if (cgiData.endRead == true && cgiData.getEndFile == false)
                {
                    cgiData.dataReady = true;
                    cgiData.status = 502;
                }
            }
                break;
        }
        else if (cgiData.getEndHeaders && cgiData.getEndFile == false)
            getBody(cgiData);
        if (cgiData.endRead == true && cgiData.contentLength > 0 && cgiData.bodyLen < cgiData.contentLength)
        {
            cgiData.dataReady = true;
            cgiData.status = 502;
            break ;
        }
        if (cgiData.lenRead == cgiData.cgBuff.length())
        {
            if ((cgiData.status == 500 || cgiData.status == 502) || cgiData.getEndFile || (pos != std::string::npos && tes.length() == 2))
                cgiData.clientState = CLIENT_CGI_ON;
            cgiData.lenRead = 0;
            break;
        }
        if (cgiData.getEndFile)
            cgiData.finFlag = true;

        if (cgiData.finFlag)
        {
            cgiData.clientState = CLIENT_CGI_ON;
            break;
        }
    }
    if (cgiData.status == 500 || cgiData.status == 502)
        cgiData.dataReady = true;
    cgiData.cgBuff.clear();
    return 0;
}
