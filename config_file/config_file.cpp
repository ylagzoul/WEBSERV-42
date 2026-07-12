#include "config_file.hpp"

Config::Config()
{
    client_max_body_size        = DEFAULT_MAX_BODY_SIZE;
    root                        = "/home";
    autoIndex                   = 0;
    methods["GET"]              = false;
    methods["POST"]             = false;
    methods["DELET"]            = false;
    delet_data["methods"]       = false;
    delet_data["error_page"]    = false;
    delet_data["index"]         = false;
    delet_data["cgiExt"]        = false;
}

void Config::check_default()
{
    if (listen_Port.size() == 0)
    {
        listen_Ip.push_back("0.0.0.0");
        listen_Port.push_back(8080);
    }
}

std::string ft_check_Path(std::string fullPath)
{
    std::stringstream ssPath(fullPath);
    std::vector<std::string> realNames;
    std::string tmp;
    std::string path;
    while (std::getline(ssPath, tmp, '/'))
    {
        if (tmp == "")
            continue;
        else
            realNames.push_back(tmp);
    }

    for (size_t i = 0; i < realNames.size(); i++)
    {
        path += '/' + realNames[i];
    }
    if (path.empty())
        path += '/';
    return path;
}

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

void Config::splite()
{
    std::string word;
    std::stringstream ss(line);
    words.clear();
    while (ss >> word)
        words.push_back(word);
    size = words.size();
}

void Config::check_Directives()
{
    if (words[0] == "listen" && blockName == "server")
        ft_listen();
    else if (words[0] == "root")
        ft_root();
    else if (words[0] == "methods")
        ft_methods();
    else if (words[0] == "error_page")
        ft_error_page();
    else if (words[0] == "client_max_body_size" && blockName == "server")
        ft_client_max_body_size();
    else if (words[0] == "upload_path")
        ft_upload_path();
    else if (words[0] == "index")
        ft_index();
    else if (words[0] == "autoindex")
        ft_autoindex();
    else if (words[0] == "return")
        ft_return();
    else if (words[0] == "authInfo")
        ft_authInfo();
    else if (words[0] == "cgi_data")
        ft_cgi_data();
    else if (words[0] == "}")
    {
        if (blockName == "location")
            blockName = "server";
    }
    else
        throw std::runtime_error("Error : Invalid Directives");
}

void ft_check_Directives(Location &obj)
{
    obj.conf.delet_data["methods"]      = true;
    obj.conf.delet_data["error_page"]   = true;
    obj.conf.delet_data["index"]        = true;
    obj.conf.delet_data["cgiExt"]       = true;
    obj.conf.delet_data["authInfo"]     = true;
}

void Webserv::config_file(std::string configNameFile)
{
    count = 0;
    std::ifstream file((configNameFile).c_str());
    if (!file)
        throw std::runtime_error("Error : opening config file");

    
    std::string readLine;
    bool emptyFile = false;
    while (getline(file, readLine))
    {
        Config configFile;
        emptyFile = true;
        configFile.line = readLine;
        configFile.splite();
        if (configFile.size == 0)
            continue;
        if ((configFile.words.size() == 1 && configFile.words[0] == "server") || (configFile.words.size() == 2 && configFile.words[0] == "server" && configFile.words[1] == "{"))
        {
            if (configFile.words.size() == 1)
            {
                getline(file, readLine);
                configFile.line = readLine;
                configFile.splite();
                if (configFile.words.size() != 1 || configFile.words[0] != "{")
                    throw std::runtime_error("Error : In opening curly brace in server");
            }
            count++;
            configFile.blockName = "server";
            while (getline(file, readLine))
            {
                configFile.line = readLine;
                configFile.splite();
                if (configFile.size == 0)
                    continue;
                else if (configFile.words[0] == "location")
                {
                    count++;
                    if (configFile.size != 3 || configFile.blockName == "location" || configFile.words[2] != "{")
                        throw std::runtime_error("Error : Location start error");
                    Location obj;
                    obj.conf = configFile;
                    ft_check_Directives(obj);
                    obj.location_name = ft_check_Path(configFile.words[1]);
                    if (configFile.sameLocation[obj.location_name] == true)
                        throw std::runtime_error("Error : Same location name");
                    configFile.sameLocation[obj.location_name] = true;
                    while (getline(file, readLine))
                    {
                        obj.conf.line = readLine;
                        obj.conf.splite();
                        if (obj.conf.size == 0)
                            continue;
                        if (obj.conf.words[0] == "}")
                        {
                            if (obj.conf.words.size() > 1)
                                throw std::runtime_error("Error : in closing curly brace in location");
                            count--;
                            break ;
                        }
                        obj.conf.check_Directives();
                    }
                    configFile.location.push_back(obj);
                }
                else
                {
                    if (configFile.words[0] == "}")
                    {
                        if (configFile.words.size() > 1)
                            throw std::runtime_error("Error : in closing curly brace in server");
                        configFile.check_default();
                        this->config.push_back(configFile);
                        count--;
                        if (count != 0)
                            throw std::runtime_error("Error : Empty config file Or {}");
                        break ;
                    }
                    configFile.check_Directives();
                }
            }
        }
        else
            throw std::runtime_error("Error : Server start error");
    }
    if (emptyFile == false)
        throw std::runtime_error("Error : Empty config file Or {}");
}
