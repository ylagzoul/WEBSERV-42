#include "../server/server.hpp"

void Config::ft_listen_prot_Ip()
{
    int         index  = 0;
    bool        foundColon = false;
    std::string port;
    std::string ip;

    while (words[1][index])
    {
        if (words[1][index] != ':' && !foundColon)
            ip += words[1][index];
        else if (words[1][index] == ':' && !foundColon)
            foundColon = true;
        else
            port += words[1][index];
        index++;
    }
    words[1] = ip;
    ft_listen_Ip();
    words[1] = port;
    ft_listen_prot();
}

void Config::ft_listen_Ip()
{
    int     sizeOctet = 0;
    int     numberPoints = 0;
    size_t  index    = 0;
    while (index < words[1].size())
    {
        if (!isdigit(words[1][index]) && words[1][index] != '.')
            throw std::runtime_error("Error : Character is not a number in an IP address.");
        else if (words[1][index] == '.' && words[1][index + 1] == '.')
            throw std::runtime_error("Error : Number of points in an IP address");
        else if (words[1][index] == '.')
        {
            numberPoints++;
            sizeOctet = 0;
        }
        else
        {
            sizeOctet = sizeOctet * 10 + (words[1][index] - '0');
            if (sizeOctet > MAXIP)
                throw std::runtime_error("Error : Exceeding the maximum of Octet IP address.");
        }
        index++;
    }
    if (numberPoints != 3)
        throw std::runtime_error("Error : Number of points in an IP address");

    listen_Ip.push_back(words[1]);
}

void Config::ft_listen_prot()
{
    size_t  index = 0;
    int     sizePort = 0;
    int     port;
    while (index < words[1].size())
    {
        if (!isdigit(words[1][index]))
            throw std::runtime_error("Error : Port is not a number.");
        sizePort = sizePort * 10 + (words[1][index] - '0');
        index++;
    }
    if (sizePort > MAXPORT)
        throw std::runtime_error("Error : Exceeding the maximum post limit");
    std::stringstream ss(words[1]);
    ss >> port;       
    listen_Port.push_back(port);
}

void Config::ft_listen()
{
    int size_s = words[1].size();
    if (size != 2 || words[1][size_s - 1] != ';')
        throw std::runtime_error("Error : Invalid listen");
    else
    {
        words[1].erase(size_s - 1);
        if (words[1].find(':') != std::string::npos)
            ft_listen_prot_Ip();
        else if (words[1].find('.') != std::string::npos)
        {
            listen_Port.push_back(8080);
            ft_listen_Ip();
        }
        else
        {
            listen_Ip.push_back("0.0.0.0");
            ft_listen_prot();
        }
    }
}
