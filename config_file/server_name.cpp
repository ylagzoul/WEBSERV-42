#include "config_file.hpp"


void Config::ft_server_name(Config& conf)
{
    if (size != 2 || conf.str[1][conf.str[1].size() - 1] != ';')
        throw std::runtime_error("error server_name");
    server_name = str[1];
}