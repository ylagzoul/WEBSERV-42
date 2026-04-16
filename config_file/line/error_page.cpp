#include "../../server/server.hpp"

void Config::ft_autoindex()
{
    int size_s = str[1].size();
    if (size != 2 || (str[1] != "off;" && str[1] != "on;"))
        throw std::runtime_error("autoindex");
    str[1].erase(size_s - 1);
    autoindex = str[1];
    if (autoindex == "on")
        autoIndex = 1;
    else
        autoIndex = 0;
}


void Config::ft_index()
{
    int size_s = str[1].size();
    int pos = size_s - 6; 
    if (size != 2 || str[1].find(".html;",pos) == std::string::npos )
        throw std::runtime_error("error in index in .html;");
    str[1].erase(size_s - 1);
    index.push_back(str[1]);
}


void Config::ft_client_max_body_size()
{
    size_t size_a = str[1].size();
    size_t i = 0;
    char c = str[1][size_a - 2];
    if (size != 2 || !ft_str_char("MGK",str[1][size_a - 2]) || str[1][size_a - 1] != ';')
        throw std::runtime_error("client_max_body_size");
    str[1].erase(size_a - 2, 2);
    while (i < str[1].size())
    {
        if (!isdigit(str[1][i]))
            throw std::runtime_error("client_max_body_size");
        i++;
    }
    client_max_body_size = atoi(str[1].c_str());
    if (c == 'G')
        client_max_body_size = client_max_body_size * 1024;
    else if (c == 'K')
        client_max_body_size = client_max_body_size / 1024;

    std::cout << client_max_body_size << std::endl;
}


void Config::ft_error_page()
{
    char c;
    int size_s = str[2].size();
    if (size != 3 || str[2][size_s - 1] != ';')
        throw std::runtime_error("error in pa");
    
    for (size_t i = 0; i < str[1].size(); i++)
    {
        c = str[1][i];
        if (!isdigit(c))
            throw std::runtime_error("error_page not digit");
    }
    str[2].erase(size_s - 1);
    error_page[std::atoi(str[1].c_str())] = str[2];
}