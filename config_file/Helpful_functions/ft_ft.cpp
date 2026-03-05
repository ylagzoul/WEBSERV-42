#include "../config_file.hpp"

bool ft_str_char(const char *str, char c)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] == c)
            return (true);
        i++;
    }
    return (false);
}