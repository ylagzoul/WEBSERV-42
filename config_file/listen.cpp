
#include "config_file.hpp"

void Config::ft_listen_prot_Ip(Config &conf)
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
        {
            ip += ';';
            n = 1;
        }
        else
            port += str[1][i];
        i++;
    }
    str[1] = ip;
    ft_listen_Ip(conf);
    str[1] = port;
    ft_listen_prot(conf);
    str[1] = ip_and_port;
}

void Config::ft_listen_Ip(Config &conf)
{
    int i = 0;
    int nums = 0;
    int n = 0;
    while (i < str[1].size() - 1 && str[1][i])
    {
        if (!isdigit(str[1][i]) && str[1][i] != '.')
            throw std::runtime_error("0-9 || '.'");
        else if (str[1][i] == '.' && ft_str_char(".;", str[1][i+1]))
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
}

void Config::ft_listen_prot(Config &conf)
{
    int i = 0;
    while (i < str[1].size()-1 && str[1][i])
    {
        if (!isdigit(str[1][i]))
            throw std::runtime_error("Element not digit");
        i++;
    }
}
void Config::ft_listen(Config &conf)
{
    int size_s = str[1].size();
    if (size != 2 || str[1][size_s - 1] != ';')
        throw std::runtime_error("error listen ");
    else
    {
        if (str[1].find(':') != std::string::npos)
            ft_listen_prot_Ip(conf);
        else if (str[1].find('.') != std::string::npos)
            ft_listen_Ip(conf);
        else
            ft_listen_prot(conf);
        str[1].erase(str[1].begin() + size_s-1);
        listen = str[1];
    }
}