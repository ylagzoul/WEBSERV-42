#include "../server/server.hpp"

void Config::ft_autoindex()
{
    int size_s = str[1].size();
    if (size != 2 || (str[1] != "off;" && str[1] != "on;"))
        throw std::runtime_error("Error : in autoindex");
    str[1].erase(size_s - 1);
    autoindex = str[1];
    if (autoindex == "on")
        autoIndex = 1;
    else
        autoIndex = 0;
}

void Config::ft_index()
{
    if (index.size() > 1)
        index.erase(index.begin(), index.end());

    if (delet_data["index"] == true)
    {
        index.clear();
    }
    size_t i = 1;
    while (i < str.size())
    {
        if (i + 1 == str.size())
            str[i].erase(str[i].size() - 1);
        index.push_back(str[i]);
        i++;
    }
    delet_data["index"] = false;
}

void Config::ft_client_max_body_size()
{
    size_t size_a = str[1].size();
    size_t i = 0;
    char c = str[1][size_a - 2];
    if (size != 2 || !ft_str_char("mMgGkKbB", str[1][size_a - 2]) || str[1][size_a - 1] != ';')
        throw std::runtime_error("Error : in client max body size");
    str[1].erase(size_a - 2, 2);
    while (i < str[1].size())
    {
        if (!isdigit(str[1][i]))
            throw std::runtime_error("Error : Element is not a number in client max body size");
        i++;
    }
    client_max_body_size = atoi(str[1].c_str());
    if (c == 'K' || c == 'k')
        client_max_body_size = client_max_body_size * 1024;
    else if (c == 'M' || c == 'm')
        client_max_body_size = client_max_body_size * 1024 * 1024;
    else if (c == 'G' || c == 'g')
        client_max_body_size = client_max_body_size * 1024 * 1024 * 1024;
}

void Config::ft_upload_path()
{
    int size_s = str[1].size();
    if (size != 2 || str[1][size_s - 1] != ';')
        throw std::runtime_error("Error : in upload path");

    str[1].erase(size_s - 1);
    uploadPath = cleanPath(str[1]);
}

void Config::ft_error_page()
{
    char c;
    int size_s = str[2].size();
    if (size != 3 || str[2][size_s - 1] != ';')
        throw std::runtime_error("Error : in error page");

    if (delet_data["error_page"] == true)
    {
        error_page.clear();
    }

    for (size_t i = 0; i < str[1].size(); i++)
    {
        c = str[1][i];
        if (!isdigit(c))
            throw std::runtime_error("Error : Element is not a number in error page");
    }
    str[2].erase(size_s - 1);
    error_page[std::atoi(str[1].c_str())] = cleanPath(str[2]);;
    delet_data["error_page"] = false;
}

void Config::ft_return()
{
    char c;
    int size_s = str[2].size();
    if (size != 3 || str[2][size_s - 1] != ';')
        throw std::runtime_error("Error : in return");

    for (size_t i = 0; i < str[1].size(); i++)
    {
        c = str[1][i];
        if (!isdigit(c))
            throw std::runtime_error("Error : Element is not a number in return");
    }
    str[2].erase(size_s - 1);
    retCode = std::atoi(str[1].c_str());
    retPath = str[2];
}