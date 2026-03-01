#include "config_file.hpp"

void Config::ft_listen(Config &conf)
{
    int i = 0;
    int a = 0;
    int k = 0;
    int c = 0;
    int w = 0;
    int nums = 0;
    int size_s = conf.str[1].size();
    if (size != 2 || conf.str[1][size_s - 1] != ';')
    {
        std::cout << "error listen "; throw erorr();
    }
    else
    {
        while (str[1][i])
        {
            if (i + 1 < size_s && ft_str_char(".:", str[1][i]) && ft_str_char(".:;", str[1][i+1]))
            {
                std::cout << ".. ";
                throw erorr();
            }
            else if (ft_str_char(".:;", str[1][i]))
            {
                if (str[1][i] == ':' && w == 0)
                    w++;
                else if (str[1][i] == ';' && k == 0)
                    k++;
                else if ((str[1][i] == ';' && k > 0 ) || (w == 0 && nums > 255 || nums < 0) || (w > 0 && ft_str_char(".:", str[1][i])))
                {
                    std::cout << "k = " << k << " w = "<< w <<  "nums ";
                    throw erorr();
                }
                if (str[1][i] == ':')
                    c++;
                a++;
                b = 0;
                nums = 0;
            }
            else if ((str[1][i]-48) < 0 || (str[1][i]-48) > 9)
            {
                std::cout << "0-9 ";
                throw erorr();
            }
            else
                nums = nums * 10 + (str[1][i] - '0');
            i++;
            if (i + 1 < size_s && str[1][i] == ';' &&  str[1][i+1] == '\0' && a == 0)
                break ;
        }
        if ((c == 0 && a != 4 && a != 0) || (c > 0 && a != 5))
        {
            std::cout << "a555 ";
            throw erorr();
        }
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
