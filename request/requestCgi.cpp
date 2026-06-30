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

bool checkDigits(CgiHandler &cgi, std::string str)
{
    std::stringstream iss(str);
    size_t value;
    iss >> value;
    
    if (!str.empty() && str.at(0) != '-' && !iss.fail() && iss.eof())
    {
        cgi.contentLength = value;
        return (true);
    }
    return (false);
}

std::string getRandomName()
{
    static size_t count;
    std::stringstream ss;
    time_t tStamp = std::time(NULL);
    struct tm *currentTime = std::localtime(&tStamp);
    char arr[9];

    std::strftime(arr, sizeof(arr), "%H%M%S", currentTime);
    std::string randName = arr;
    ss << count++;
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

bool isValidStatus(std::string str)
{
    int count = 0;
    for (size_t i = 0; i < 3; i++)
    {
        if (isdigit(str.at(i)))
            count++;
    }
    // cout << count << std::endl;
    if (count != 3)
        return (false);
    else
        return (true);
}

bool    parseLine(std::string str, CgiHandler &cgiData)
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

void getHeadersCgi(CgiHandler &cgiData)
{
    size_t endLine = cgiData.fullLine.find("\r\n");
    if (endLine == std::string::npos)
    {
        if (cgiData.getMove == 2)
            cgiData.getMove = 3;
        return;
    }
    std::string line = cgiData.fullLine.substr(0, endLine);
    if (line.empty())
    {
        cgiData.getEndHeaders = true;
        cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
        cgiData.fullLine.erase(0, endLine + 2);
        std::map<std::string, std::string>::iterator it;
        it = cgiData.cgHeaders.find("transfer-encoding");
        if (cgiData.cgHeaders.find("content-type") == cgiData.cgHeaders.end())
            return(returnedBadError(cgiData));
        if (it != cgiData.cgHeaders.end() && it->second == "chunked")
            cgiData.bodyType = CHUNKED;
        if (cgiData.cgHeaders.find("status") != cgiData.cgHeaders.end())
        {
            if (parseLine(cgiData.cgHeaders["status"], cgiData) == false)
                return(returnedBadError(cgiData));
        }
        return;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (!isPrintableC(key) || !isHoldSpaceCgi(key) || key.empty())
            return(returnedBadError(cgiData));
        value = trimCgi(value);
        // cout << key << " : " << value << std::endl;
        if (cgiData.cgHeaders.find(key) != cgiData.cgHeaders.end() && key == "content-length")
            return(returnedBadError(cgiData));
        cgiData.cgHeaders[tolowerStri(key)] = value;
        if (cgiData.cgHeaders.find("content-length") != cgiData.cgHeaders.end() && !checkDigits(cgiData, cgiData.cgHeaders["content-length"]))
            return(returnedBadError(cgiData));
    }
    else
        return(returnedBadError(cgiData));
    cgiData.lenRead += cgiData.fullLine.substr(0, endLine + 2).length();
    cgiData.countLenHead += cgiData.fullLine.substr(0, endLine + 2).length();
    cgiData.fullLine.erase(0, endLine + 2);
    if (cgiData.countLenHead >= MAX_LEN_HEADER_FEILD)
        return(returnedBadError(cgiData));
    return;
}

void getBody(CgiHandler &cgiData)
{
    cgiData.lenRead += cgiData.fullLine.length();

    if (cgiData.fd == -1)
    {
        cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("clientCgi_") + getRandomName();
        cgiData.bodyLen = 0;
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
            return(returnedInternalError(cgiData));
    }
    cgiData.bodyLen += cgiData.fullLine.length();
    if (cgiData.bodyLen > DEFAULT_MAX_BODY_SIZE )
        return(returnedBadError(cgiData));
    if (cgiData.endRead == false)
    {
        if (cgiData.bodyLen != 0)
        {
            if (write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length()) == -1)
                return (returnedInternalError(cgiData));
            cgiData.fullLine.clear();
        }
        return ;
    }
}

void getChunked(CgiHandler &cgiData)
{
    size_t endTxt;
    size_t endNum;
    if (cgiData.fd == -1)
    {
        cgiData.cgBodyFileName = BODY_FILE_PATH + std::string("clientCgi_") + getRandomName();
        cgiData.bodyLen = 0;
        cgiData.fd = open(cgiData.cgBodyFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (cgiData.fd == -1)
            return(returnedInternalError(cgiData));
    }
    endNum = cgiData.fullLine.find("\r\n");
    if (endNum != std::string::npos)
        endTxt = cgiData.fullLine.find("\r\n", endNum + 2);
    while (endNum != std::string::npos && endTxt != std::string::npos)
    {
        char *end = NULL;
        std::string chunkNum  = cgiData.fullLine.substr(0, endNum);
        std::string chunkData = cgiData.fullLine.substr(endNum + 2, endTxt - endNum - 2);
        cgiData.lenRead += cgiData.fullLine.substr(0, endTxt + 2).length();
        double chunkSize = std::strtol(chunkNum.c_str(), &end, 16);
        if (errno == ERANGE || *end)
            return(returnedBadError(cgiData));
        if (chunkSize == 0 && chunkData.empty())
        {
            // cout << "BOOYAH\n";
            if (write(cgiData.fd, chunkData.c_str(), chunkSize) == -1)
                return(returnedInternalError(cgiData));
            cgiData.fullLine.erase(0, endTxt + 2);
            cgiData.getEndFile = true;
            return ;
        }
        if (chunkData.length() < chunkSize)
            return(returnedBadError(cgiData));
        if (write(cgiData.fd, chunkData.c_str(), chunkSize) == -1)
                return(returnedInternalError(cgiData));
        cgiData.fullLine.erase(0, endTxt + 2);
        endNum = cgiData.fullLine.find("\r\n");
        if (endNum != std::string::npos)
            endTxt = cgiData.fullLine.find("\r\n", endNum + 2);
    }
    if (cgiData.getMove == 4)
        cgiData.getMove = 3;
}


int parseCgiResponse(CgiHandler &cgiData)
{
    size_t pos;
    std::string tes;
    cgiData.fullLine.append(cgiData.cgBuff.c_str(), cgiData.cgBuff.length());
    if (cgiData.getMove == 3)
        cgiData.getMove = 0;
    while (1)
    {
        pos = cgiData.fullLine.find("\r\n");
        if (pos == std::string::npos && cgiData.getEndHeaders == false)
            break ;
        tes = cgiData.fullLine;
        if (cgiData.getEndHeaders == false) // FIX the /r/n
        {
            getHeadersCgi(cgiData);
            if (cgiData.status == 500 || cgiData.status == 502)
                cgiData.clientState = CLIENT_CGI_ON ;
            // cout << cgiData.getMove << std::endl;
            if (cgiData.getMove == 3)
                break ;
            cgiData.getMove = 2;
        }
        if (cgiData.getEndHeaders && cgiData.getEndFile == false && cgiData.bodyType == CHUNKED)
        {
            getChunked(cgiData);
            if (cgiData.getMove == 3)
                break;
        }
        else if (cgiData.getEndHeaders && cgiData.getEndFile == false)
            getBody(cgiData);
        cgiData.getMove = 4;
        if (cgiData.lenRead == cgiData.cgBuff.length())
        {
            if ((cgiData.status == 500 || cgiData.status == 502) || cgiData.getEndFile || (pos != std::string::npos && tes.length() == 2))
                cgiData.clientState = CLIENT_CGI_ON;
            cgiData.lenRead = 0;
            break ;
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
