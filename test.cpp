
#include "response/CgiHandler.hpp"

CgiHandler::CgiHandler() : lenRead(0), bodyFd(-1), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_OF), reqState(DEFAULT_ST) {};



std::string tolowerStr(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}

bool isPrintable(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!isprint(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

std::string toStr(int nb)
{
    std::stringstream ss;
    ss << nb;
    return (ss.str());
}

bool isHex(char h)
{
    return ( h >= '0' && h <= '9' ) ||
           ( h >= 'a' && h <= 'f' ) ||
           ( h >= 'A' && h <= 'F' ); 
}

int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    return (-1);
}

char hexToChar(char a, char b)
{
    int high = hexValue(a);
    int low  = hexValue(b);

    return ((high << 4) | low);
}

bool checkPrecentEnc(std::string &pathSlice)
{
    std::string copyPath = pathSlice;
    pathSlice.clear();
    for (size_t i = 0; i < copyPath.length(); i++)
    {
        if (copyPath.at(i) == '%')
        {
            // std::cout << "Found % at index: " << i << std::endl;
            if(!copyPath[i + 1] || !copyPath[i + 2])
                return (false);
            if (!isHex((char)copyPath.at(i+1)) || !isHex((char)copyPath.at(i+2)))
                return(false);
            char res = hexToChar(copyPath.at(i+1), copyPath.at(i+2));
            if ((unsigned char)res > 127)
                return false;
            pathSlice += res;
            i += 2;
        }
        else
        {
            pathSlice += copyPath.at(i);
        }
    }
    return (true);
}

bool checkPath(std::string &path)
{
    std::stringstream iss(path);
    std::string splitedPath;
    std::string tmp;

    if (path.at(0) != '/')
        return (false);
    else if (path.at(0) == '/' && path.length() == 1)
        return (true);
    while (getline(iss, splitedPath, '/'))
    {
        if (!splitedPath.empty())
        {
            // cout << splitedPath << std::endl;
            if(!isPrintable(splitedPath))
                return (false);
            if (!checkPrecentEnc(splitedPath))
                return (false);
        }
        tmp += "/";
        tmp += splitedPath;
    }
    path = tmp;
    return (true);
}

bool isLfCr(const std::string &str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (isspace(static_cast<unsigned char>(str[i])))
            return false;
    }
    return (true);
}

void fillCheck(Request &req, std::string reqLine)
{
    std::stringstream splitReq(reqLine);
    std::string slice;
    std::vector<std::string> array;
    while (splitReq >> slice)
        array.push_back(slice);
    if (array.size() != 3)
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    if (array[2] != "HTTP/1.1" && array[2] != "HTTP/1.0")
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
    req.method = array[0];
    req.path = array[1];
    req.httpVersion = array[2];
    if (!checkPath(req.path))
    {
        req.status = 400;
        req.finFlag = true;
        return;
    }
}

void getRequestLine(Request &req)
{
    size_t pos = req.fullLine.find("\r\n");
    if (pos == std::string::npos)
        return;
    fillCheck(req, req.fullLine.substr(0, pos));
    req.lenRead += req.fullLine.substr(0, pos + 2).length();
    req.fullLine.erase(0, pos + 2);
    // cout << req.method << " " << req.httpVersion << " " << req.path << std::endl; 
    // cout << "size : " << req.fullLine.size() << " content = "<< req.fullLine << std::endl;
}

void getHeaders(CgiHandler &cgiData)
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
        std::map<std::string, std::string>::iterator it;
        it = cgiData.cgHeaders.find("transfer-encoding");
        if (cgiData.cgHeaders.find("host") != cgiData.cgHeaders.end())
            cgiData.host = cgiData.cgHeaders["host"];
        if (it != cgiData.cgHeaders.end() && it->second == "chunked")
            cgiData.bodyType = CHUNKED;
        else if (cgiData.cgHeaders.find("content-length") != cgiData.cgHeaders.end())
        {
            cgiData.contentLength = std::atoi(cgiData.cgHeaders["content-length"].c_str());
            cgiData.bodyType = FIXED;
        }
        else if (cgiData.method != "GET")
        {
            cgiData.status = 400;
            cgiData.finFlag = true;
            return;
        }
        return;
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos)
    {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
        if (!isPrintable(key))
        {
            req.status = 400;
            req.finFlag = true;
            return;
        }
        value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
        if (!isLfCr(value)) //useless
        {
            req.status = 400;
            req.finFlag = true;
            return;
        }
        req.headers[tolowerStr(key)] = value;
    }
    req.lenRead += req.fullLine.substr(0, endLine + 2).length();
    req.fullLine.erase(0, endLine + 2);
    return;
}

// void getChunkedBody(Request &req)
// {
//     (void)fd;
//     std::vector<int> nb;
//     if (req.fdFile == -1)
//     {
//         req.fdFile = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
//         if (req.fdFile == -1)
//             throw std::runtime_error("Error file");
//     }
//     while (true)
//     {
//         if (std::strtol(req.fullLine.c_str(), NULL, 16) == 0 && req.fullLine.length() == 1)
//             req.fullLine.append("\n");
//         size_t pos = req.fullLine.find("\n");
//         if (pos == std::string::npos)
//         {
//             break ;
//         }
//         std::string sizeStr = req.fullLine.substr(0, pos);
//         size_t finalRead = req.fullLine.find("0\n\n");
//         // cout << finalRead << "=>" << req.fullLine << std::endl;
//         req.fullLine.erase(0, pos + 1);

//         int chunkSize = std::strtol(sizeStr.c_str(), NULL, 16);
//         nb.push_back(chunkSize);
//         if (chunkSize == 0)
//         {
//             if (finalRead == std::string::npos)
//             {
//                 req.status = 400;
//                 close(req.fdFile);
//                 throw std::runtime_error("Bad request");
//             }
//             req.fullLine.erase(0, 1);
//             req.getEndFile = true;
//             close(req.fdFile);
//             return ;
//         }
//         size_t pos2 = req.fullLine.find("\n");
//         std::string subStr = req.fullLine.substr(0, pos2);
//         if (subStr.length() < (size_t)chunkSize)
//         {
//             req.status = 400;
//             close(req.fdFile);
//             throw std::runtime_error("Bad request");
//         }
//         std::string data = req.fullLine.substr(0, chunkSize);
//         write(req.fdFile, data.c_str(), data.size());
//         req.fullLine.erase(0, pos2 + 1);
//         if (chunkSize != 0)
//             readRequest(req, fd);
//     }
//     // I Have new logic is made Here ;
//     int i = 0;
//     while (i < nb.size())
//     {
//         if (nb[i] == 0)
//             break;
//         i++;
//     }
//     if ( i != nb.size() )
//         readRequest(req, fd);
// }

void getFixedBody(Request &req, Config con)
{
    (void)con;
    static size_t bodyLen;
    req.lenRead += req.fullLine.length();
    if (req.bodyFd == -1)
    {
        req.bodyFd = open(req.bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (req.bodyFd == -1)
        {
            std::cerr << "Error opening file\n";
            req.status = 500;
            return ;
        }
    }
    bodyLen += req.fullLine.length();
    if (bodyLen > 10000000) // after handel
    {
        req.status = 400;
        req.getEndFile = true;
        return;
    }
    if (bodyLen < req.contentLength)
    {
        if (bodyLen != 0)
        {
            write(req.bodyFd, req.fullLine.c_str(), req.fullLine.length());
            req.fullLine.clear();
        }
        return;
    }
    int sizePa = (req.contentLength + req.fullLine.length()) - bodyLen;
    std::string d = req.fullLine.substr(0, sizePa);

    write(req.bodyFd, d.c_str(), sizePa);
    close(req.bodyFd);
    req.getEndFile = true;
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
    cgiData.fullLine.append(cgiData.cgBuff, cgiData.cgBuff.length());
    cgiData.bodyPath = BODY_FILE_PATH + std::string("cgiCleint") + toStr(1);
    while (1)
    {
        if (cgiData.getEndHeaders == false)
            getHeaders(cgiData);
        if (cgiData.getEndHeaders == false)
            getHeaders(cgiData);
        else
            cgiData.reqState = SECOND_ST;
        if (req[userId].method == "POST" && req[userId].getEndFile == false && req[userId].bodyType == FIXED)
            getFixedBody(req[userId], con);
        if (req[userId].lenRead == len)
        {
            if ((req[userId].status == 400) || (req[userId].getEndFile) || (req[userId].method != "POST" && pos != std::string::npos) )
                req[userId].clientState = CLIENT_ON;
            req[userId].lenRead = 0;    
            break ;
        }    
        // else if (req.method == "POST" && req.getEndFile == false && req.bodyType == CHUNKED)
        //     getChunkedBody(req);
        if ((req[userId].getEndFile) || (req[userId].method != "POST" && !req[userId].method.empty()))
            req[userId].finFlag = true;

        if (req[userId].finFlag)
        {
            cout << "SFSFSFSF\n";
            req[userId].reqState = SECOND_ST;
            req[userId].clientState = CLIENT_ON;
            break;
        }
    }
    return;







    cgiData.cgBuff.clear();
    return 0;
}

int main()
{

}