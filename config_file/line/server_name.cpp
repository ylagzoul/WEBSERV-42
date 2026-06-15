#include "../../server/server.hpp"

void Config::ft_methods()
{
    if (delet_data["methods"] == true)
    {
        methods.clear();
    }
    methods["GET"] = false;
    methods["POST"] = false;
    methods["DELETE"] = false;
    int size_s = str[size - 1].size();
    str[size - 1].erase(size_s - 1);
    int i = 1;
    while (i < size)
    {
        if (str[i] == "GET" && methods["GET"] == false)
        {
            methods["GET"] = true;
        }
        else if (str[i] == "POST" && methods["POST"] == false)
        {
            methods["POST"] = true;
        }
        else if (str[i] == "DELETE" && methods["DELETE"] == false)
        {
            methods["DELETE"] = true;
        }
        else
            throw std::runtime_error("invalid method");
        i++;
    }
    delet_data["methods"] = false;
}

void Config::ft_root()
{
    int size_s = str[1].size();
    if (size != 2 || str[1][0] != '/' || str[1][size_s - 1] != ';')
        throw std::runtime_error("eroor in ./");
    str[1].erase(str[1].begin() + size_s - 1);
    root = str[1];
}

void Config::ft_server_name()
{
    // for (int j = 0;j < size;j++)
    // {
    //     server_name.push_back(str[j]);
    // }
    int lenth = str[1].size();
    char c;
    int a = 0;

    if (str[1][0] == '*')
    {
        if (lenth < 3 || str[1][1] != '.')
            throw std::runtime_error("error server_name2");
        a = 2;
    }

    if (size != 2 || str[1][lenth - 1] != ';' || ft_str_char(".123456789-", str[1][0]) || str[1][lenth - 2] == '.' || lenth > 253)
        throw std::runtime_error("error server_name");

    for (int i = a; i < lenth - 1; i++)
    {
        c = str[1][i];
        if (!std::isalnum(c) && c != '-' && c != '.')
            throw std::runtime_error("error name");
        else if (i < lenth - 1 && ft_str_char(".-", c) && ft_str_char(".-", str[1][i + 1]))
            throw std::runtime_error("error name1");
    }
    str[1].erase(str[1].begin() + lenth - 1);
    server_name = str[1];
}