#include "config_file.hpp"

void Config::ft_listen(Config &conf)
{
    if (size != 2 || conf.str[1][conf.str[1].size() - 1] != ';')
    {
        std::cout << "error listen\n";
        throw erorr();
    } 
    listen = conf.str[1];
}

void Config::ft_server_name(Config& conf)
{
    if (size != 2 || conf.str[1][conf.str[1].size() - 1] != ';')
    {
        std::cout << "error server_name\n";
        throw erorr();
    }
    server_name = str[1];
}
