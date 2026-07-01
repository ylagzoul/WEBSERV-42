#include "../server/server.hpp"



void Config::ft_listen_prot_Ip()
{
    int i = 0;
    int n = 0;
    std::string port;
    std::string ip;
    std::string ip_and_port;
    ip_and_port = str[1];
    while (str[1][i])
    {
        if (str[1][i] != ':' && n == 0)
            ip += str[1][i];
        else if (str[1][i] == ':' && n == 0)
            n = 1;
        else
            port += str[1][i];
        i++;
    }
    str[1] = ip;
    ft_listen_Ip();
    str[1] = port;
    ft_listen_prot();
}

void Config::ft_listen_Ip()
{
    size_t i = 0;
    int nums = 0;
    int n = 0;
    while (i < str[1].size() && str[1][i])
    {
        if (!isdigit(str[1][i]) && str[1][i] != '.')
            throw std::runtime_error("Error : Element is not a number in an IP address.");
        else if (str[1][i] == '.' && ft_str_char(".", str[1][i+1]))
            throw std::runtime_error("Error : Number of points in an IP address");
        else if (str[1][i] == '.')
        {
            n++;
            nums = 0;
        }
        else
        {
            nums = nums * 10 + (str[1][i] - '0');
            if (nums > MAXIP)
                throw std::runtime_error("Error : Exceeding the maximum IP address limit");
        }
        i++;
    }
    if (n != 3)
        throw std::runtime_error("Error : Number of points in an IP address");

    listen_Ip.push_back(str[1]);
}

void Config::ft_listen_prot()
{
    size_t i = 0;
    int port;
    while (i < str[1].size() && str[1][i])
    {
        if (!isdigit(str[1][i]))
            throw std::runtime_error("Error :  Element is not a number in an post.");
        i++;
    }
    if (atoi(str[1].c_str()) > MAXPORT)
        throw std::runtime_error("Error : Exceeding the maximum post limit");
    std::stringstream ss(str[1]);
    ss >> port;       
    listen_Port.push_back(port);
}

void Config::ft_listen()
{
    int size_s = str[1].size();
    if (size != 2 || str[1][size_s - 1] != ';' || str[1] == ";")
        throw std::runtime_error("Error : Invalid listen");
    else
    {
        str[1].erase(size_s-1);
        if (str[1].find(':') != std::string::npos)
            ft_listen_prot_Ip();
        else if (str[1].find('.') != std::string::npos)
        {
            listen_Port.push_back(80);
            ft_listen_Ip();
        }
        else
        {
            listen_Ip.push_back("0.0.0.0");
            ft_listen_prot();
        }
    }
}
