#include "config_file.hpp"


void Config::ft_server_name(Config& conf)
{
    int lenth = str[1].size();
    char c;
    int a = 0;

    if (str[1][0] == '*')
    {
        if (lenth < 3 || str[1][1] != '.')
            throw std::runtime_error("error server_name2");
        a = 2;
    }

    if (size != 2 || str[1][lenth - 1] != ';' || ft_str_char(".123456789-",str[1][0])
        || str[1][lenth - 2] == '.' || lenth > 253)
        throw std::runtime_error("error server_name");

    for (int i = a; i < lenth - 1; i++)
    {
        c = str[1][i];
        if (!std::isalnum(c) && c != '-' && c != '.')
            throw std::runtime_error("error name");
        else if (i < lenth -1 && ft_str_char(".-", c) && ft_str_char(".-",str[1][i+1]))
            throw std::runtime_error("error name1");
    }
    server_name = str[1];
}