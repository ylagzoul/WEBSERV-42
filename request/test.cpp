
// 



#include "response/CgiHandler.hpp"

CgiHandler::CgiHandler() : fd(-1), cgStatusCode(200), cgStatusName("OK"), lenRead(0), getEndHeaders(false), finFlag(false), getEndFile(false), clientState(CLIENT_CGI_OF), reqState(DEFAULT_CGI_ST), endRead(false)
{
    dataReady = false;
    cgiValid = false;
    canRead = false;
};

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
        if (cgiData.cgHeaders.find("content-type") == cgiData.cgHeaders.end())
        {
            cgiData.cgStatusCode = 502;
            cgiData.cgStatusName = "Bad Gateway";
            cgiData.finFlag = true;
            return;
        }
        if (cgiData.cgHeaders.find("status") != cgiData.cgHeaders.end())
            parseLine(cgiData.cgHeaders["status"], cgiData);
        // cgiData.contentLength = std::atoi(cgiData.cgHeaders["content-length"].c_str());
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
        bodyLen = 0;
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
        cgiData.cgStatusCode = 502;
        cgiData.getEndFile = true;
        return ;
    }
    if (cgiData.endRead == false)
    {
        if (bodyLen != 0)
        {
            write(cgiData.fd, cgiData.fullLine.c_str(), cgiData.fullLine.length());
            cgiData.fullLine.clear();
        }
        std::cout << "Khrejt\n\n";
        return ;
    }
    close(cgiData.fd);
    std::cout << "ENd file true\n\n";
    cgiData.getEndFile = true;
    // cgiData.endRead = true;
}

int parseCgaadasdiResponse(CgiHandler &cgiData)
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
        if (cgiData.getEndHeaders && cgiData.getEndFile == false)
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

// int main()
// {
//     CgiHandler cgi;
//     std::string buffer;
//     buffer =
//     "Content-Type: text/html\r\n"
//     "Content-Length: 1000\r\n"
//     "Set-Cookie: sessionId=abc123xyz789; Path=/; HttpOnly\r\n"
//     "X-Powered-By: custom-cgi-engine\r\n"
//     "Cache-Control: no-cache\r\n"
//     "Status: 404 Not Found\r\n"
//     "Content-Type: text/html\r\n"
//     "X-Error-Code: FILE_MISSI\r\n"
//     "\r\n"
//     "<!DOCTYPE html>\n"
//     "<html lang=\"en\">\n"
//     "<head>\n"
//     "    <title>User Dashboard</title>\n"
//     "    <meta charset=\"UTF-8\">\n"
//     "</head>\n"
//     "<body>\n"
//     "    <h1>Welcome back, Abdessamad</h1>\n"
//     "\n"
//     "    <div class=\"profile\">\n"
//     "        <h2>Account Information</h2>\n"
//     "        <ul>\n"
//     "            <li>User ID: 48291</li>\n"
//     "            <li>Plan: Premium</li>\n"
//     "            <li>Status: Active</li>\n"
//     "            <li>Last Login: Tue May 12 2026 09:14:22 GMT</li>\n"
//     "        </ul>\n"
//     "    </div>\n"
//     "\n"
//     "    <div class=\"notifications\">\n"
//     "        <h2>Notifications</h2>\n"
//     "        <p>You have 5 unread messages.</p>\n"
//     "        <p>Your storage usage is 73%.</p>\n"
//     "    </div>\n"
//     "\n"
//     "    <div class=\"recent-files\">\n"
//     "        <h2>Recent Files</h2>\n"
//     "        <table border=\"1\">\n"
//     "            <tr>\n"
//     "                <th>Filename</th>\n"
//     "                <th>Size</th>\n"
//     "                <th>Modified</th>\n"
//     "            </tr>\n"
//     "            <tr>\n"
//     "                <td>report.pdf</td>\n"
//     "                <td>1.2 MB</td>\n"
//     "                <td>2026-05-11</td>\n"
//     "            </tr>\n"
//     "            <tr>\n"
//     "                <td>notes.txt</td>\n"
//     "                <td>14 KB</td>\n"
//     "                <td>2026-05-10</td>\n"
//     "            </tr>\n"
//     "            <tr>\n"
//     "                <td>archive.tar.gz</td>\n"
//     "                <td>92 MB</td>\n"
//     "                <td>2026-05-08</td>\n"
//     "            </tr>\n"
//     "        </table>\n"
//     "    </div>\n"
//     "</body>\n"
//     "</html>\n";
//     cgi.cgBuff = buffer;
//     // while(cgi.clientState != CLIENT_CGI_ON)
//     // {
//         parseCgiResponse(cgi);
//     // }
//     std::map<std::string, std::string>::iterator it;

//     for (it = cgi.cgHeaders.begin(); it != cgi.cgHeaders.end(); ++it)
//     {
//         std::cout << it->first << ":" << it->second << std::endl;
//     }   
//     std::cout << "-->" << cgi.cgStatusCode << std::endl;
//     std::cout << "--> " << cgi.cgStatusName << std::endl;
//     std::cout << "--> " << cgi.clientState << std::endl;


// }

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
# define KB 1024

int main()
{
    int fd = open("/home/mradouan/goinfre/dfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd==-1)
        cout << "kayn chi error a wld 3ammi\n";
    int i = 0;
    string buff(10000, 'e');
    while (i < KB * KB)
    {
        write(fd, buff.c_str() , 10000);
        i++;
    }
}