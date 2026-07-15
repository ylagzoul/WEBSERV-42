#include "../server/server.hpp"

void Config::ft_cgi_data()
{
    if (delet_data["cgiExt"] == true)
    {
        cgiExt.clear();
        cgiPath.clear();
    }
    for (size_t i = 0; i < cgiExt.size(); i++)
    {
        if (cgiExt[i] == words[1])
            throw std::runtime_error("Error : in cgiExt = " + words[1]);
    }
    if (words.size() != 3 || words[1][0] != '.' || words[2][words[2].size() - 1] != ';')
        throw std::runtime_error("Error : in cgi data");

    words[2].erase(words[2].size() - 1);
    cgiExt.push_back(words[1]);
    cgiPath.push_back(ft_check_Path(words[2]));
    delet_data["cgiExt"] = false;
}

void Config::ft_authInfo()
{
    if (delet_data["authInfo"] == true)
    {
        loginToken.clear();
        redirectTokens.clear();
    }
    if (words.size() < 3 || words[words.size() - 1][words[words.size() - 1].size() - 1] != ';')
        throw std::runtime_error("Error : in loginToken in authInfo");
    loginToken = ft_check_Path(words[1]);
    for (size_t i = 2; i < words.size(); i++)
    {
        redirectTokens.push_back(ft_check_Path(words[i]));
    }
    redirectTokens[redirectTokens.size() - 1].erase(redirectTokens[redirectTokens.size() - 1].size() - 1);
    delet_data["authInfo"] = false;
}


void Config::ft_return()
{
    char c;
    int size_s = words[2].size();
    if (size != 3 || words[2][size_s - 1] != ';')
        throw std::runtime_error("Error : in return");

    for (size_t i = 0; i < words[1].size(); i++)
    {
        c = words[1][i];
        if (!isdigit(c))
            throw std::runtime_error("Error : Element is not a number in return");
    }
    words[2].erase(size_s - 1);
    retCode = std::atoi(words[1].c_str());
    retPath = words[2];
}

void Config::ft_autoindex()
{
    if (size != 2 || (words[1] != "off;" && words[1] != "on;"))
        throw std::runtime_error("Error : in autoindex");
    if (words[1] == "on;")
        autoIndex = 1;
    else
        autoIndex = 0;
}

void Config::ft_index()
{
    if (delet_data["index"] == true)
        index.clear();
    
    if (words[words.size()-1][words[words.size()-1].size() - 1] != ';')
        throw std::runtime_error("Error : in Index");
    size_t i = 1;
    while (i < words.size())
    {
        if (i + 1 == words.size())
            words[i].erase(words[i].size() - 1);
        checkValid(words[i]);
        index.push_back(words[i]);
        i++;
    }
    delet_data["index"] = false;
}

void Config::ft_upload_path()
{
    int size_s = words[1].size();
    if (size != 2 || words[1][size_s - 1] != ';')
        throw std::runtime_error("Error : in upload path");

    words[1].erase(size_s - 1);
    uploadPath = ft_check_Path(words[1]);
}

void Config::ft_client_max_body_size()
{
    size_t size_a = words[1].size();
    size_t index = 0;
    char c = words[1][size_a - 2];
    if (size != 2 || !ft_str_char("mMgGkKbB", words[1][size_a - 2]) || words[1][size_a - 1] != ';')
        throw std::runtime_error("Error : in client max body size");
    words[1].erase(size_a - 2, 2);
    while (index < words[1].size())
    {
        if (!isdigit(words[1][index]))
            throw std::runtime_error("Error : Element is not a number in client max body size");
        index++;
    }
    client_max_body_size = std::atoi(words[1].c_str());
    if (c == 'K' || c == 'k')
        client_max_body_size = client_max_body_size * 1024;
    else if (c == 'M' || c == 'm')
        client_max_body_size = client_max_body_size * 1024 * 1024;
    else if (c == 'G' || c == 'g')
        client_max_body_size = client_max_body_size * 1024 * 1024 * 1024;
}

void Config::ft_error_page()
{
    char c;
    int size_s = words[2].size();
    if (size != 3 || words[2][size_s - 1] != ';')
        throw std::runtime_error("Error : in error page");

    if (delet_data["error_page"] == true)
        error_page.clear();

    for (size_t index = 0; index < words[1].size(); index++)
    {
        c = words[1][index];
        if (!isdigit(c))
            throw std::runtime_error("Error : Element is not a number in error page");
    }
    words[2].erase(size_s - 1);
    error_page[std::atoi(words[1].c_str())] = ft_check_Path(words[2]);
    delet_data["error_page"] = false;
}

void Config::ft_methods()
{
    if (delet_data["methods"] == true)
        methods.clear();
    
    methods["GET"]    = false;
    methods["POST"]   = false;
    methods["DELETE"] = false;
    int size_s = words[size - 1].size();
    if (words[size - 1][size_s-1] != ';')
        throw std::runtime_error("Error : Invalid methods");
    words[size - 1].erase(size_s - 1);
    int index = 1;
    while (index < size)
    {
        if (words[index] == "GET")
            methods["GET"] = true;
        else if (words[index] == "POST")
            methods["POST"] = true;
        else if (words[index] == "DELETE")
            methods["DELETE"] = true;
        else
            throw std::runtime_error("Error : Invalid methods");
        index++;
    }
    delet_data["methods"] = false;
}

void Config::ft_root()
{
    int sizePath = words[1].size();
    if (size != 2 || words[1][sizePath - 1] != ';')
        throw std::runtime_error("Error : in root");

    words[1].erase(words[1].begin() + sizePath - 1);
    root = ft_check_Path(words[1]);
}
