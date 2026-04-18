#include "../../server/server.hpp"

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
            throw std::runtime_error("0-9");
        else if (str[1][i] == '.' && ft_str_char(".", str[1][i+1]))
            throw std::runtime_error("..");
        else if (str[1][i] == '.')
        {
            n++;
            if (nums > 255)
                throw std::runtime_error("big rang 256");
            nums = 0;
        }
        else
            nums = nums * 10 + (str[1][i] - '0');
        i++;
    }
    if (n != 3)
        throw std::runtime_error("big");
    listen_Ip.push_back(str[1]);
}

void Config::ft_listen_prot()
{
    size_t i = 0;
    while (i < str[1].size() && str[1][i])
    {
        if (!isdigit(str[1][i]))
            throw std::runtime_error("Element not digit");
        i++;
    }
    if (atoi(str[1].c_str()) > 65535)
        throw std::runtime_error("invalid max port");
    listen_Port.push_back(str[1]);
}
void Config::ft_listen()
{
    int size_s = str[1].size();
    if (size != 2 || str[1][size_s - 1] != ';')
        throw std::runtime_error("error listen ");
    else
    {
        str[1].erase(size_s-1);
        if (str[1].find(':') != std::string::npos)
            ft_listen_prot_Ip();
        else if (str[1].find('.') != std::string::npos)
        {
            listen_Port.push_back("80");
            ft_listen_Ip();
        }
        else
        {
            listen_Ip.push_back("0.0.0.0");
            ft_listen_prot();
        }
    }
}