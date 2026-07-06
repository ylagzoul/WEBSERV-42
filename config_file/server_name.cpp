#include "../server/server.hpp"

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
            throw std::runtime_error("Erorr : Invalid Element in method");
        i++;
    }
    delet_data["methods"] = false;
}

void Config::ft_root()
{
    int size_s = str[1].size();
    if (size != 2 || str[1][size_s - 1] != ';')
        throw std::runtime_error("Erorr : in root");

    str[1].erase(str[1].begin() + size_s - 1);
    root = cleanPath(str[1]);
}
