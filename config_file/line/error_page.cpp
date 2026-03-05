#include "../config_file.hpp"

void Config::ft_error_page()
{
    char c;
    int size_s = str[2].size();
    if (size != 3 || str[2][size_s - 1] != ';')
        throw std::runtime_error("error in pa");
    
    for (size_t i = 0; i < str[1].size() -1; i++)
    {
        c = str[1][i];
        if (!isdigit(c))
            throw std::runtime_error("error_page not digit");
    }
    str[2].erase(size_s-1);
    error_page[std::atoi(str[1].c_str())] = str[2];
}