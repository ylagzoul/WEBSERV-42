#include "../server/server.hpp"

Config::Config()
{
    delet_data["methods"] = false;
    delet_data["error_page"] = false;
    delet_data["index"] = false;
    delet_data["cgiExt"] = false;
    // emptyFile = false;
    // server_name = "";
    root = "/home";
    autoIndex = 0;
    client_max_body_size = DEFAULT_MAX_BODY_SIZE;

    methods["GET"] = false;
    methods["POST"] = false;
    methods["DELET"] = false;
}

void Config::splite()
{
    std::string word;
    std::stringstream ss(line);
    str.clear();
    while (ss >> word)
        str.push_back(word);
    size = str.size();
}

void Config::check_default()
{
    if (listen_Port.size() == 0)
    {
        listen_Ip.push_back("0.0.0.0");
        listen_Port.push_back(80);
    }
}

void Config::ft_authInfo()
{
    if (delet_data["authInfo"] == true)
    {
        loginToken.clear();
        redirectTokens.clear();
    }
    if (str.size() < 3 || str[1][0] != '/' || str[str.size() - 1][str[str.size() - 1].size() - 1] != ';')
        throw std::runtime_error("loginToken");
    loginToken = str[1];
    for (size_t i = 2; i < str.size(); i++)
    { 
        if (str[i][0] != '/')
            throw std::runtime_error("redirectTokens");
        redirectTokens.push_back(str[i]);
    }
    redirectTokens[redirectTokens.size() - 1].erase(redirectTokens[redirectTokens.size() - 1].size() - 1);
    delet_data["authInfo"] = false;
}

void Config::ft_cgi_data()
{
    if (delet_data["cgiExt"] == true)
    {
        cgiExt.clear();
        cgiPath.clear();
    }
    for (size_t i = 0; i < cgiExt.size(); i++)
    {
        if (cgiExt[i] == str[1])
            throw std::runtime_error("error in cgi_ data " + str[1]);
    }
    if (str.size() != 3 || str[1][0] != '.' || str[2][str[2].size() - 1] != ';')
        throw std::runtime_error("ft_cgi_data");

    if (delet_data["cgiExt"] == true)
    {
        cgiExt.clear();
        cgiPath.clear();
    }

    str[2].erase(str[2].size() - 1);
    cgiExt.push_back(str[1]);
    cgiPath.push_back(str[2]);
    delet_data["cgiExt"] = false;
}

void Config::function()
{
    if (str[0] == "listen" && blockName == "server")
        ft_listen();
    else if (str[0] == "server_name" && blockName == "server")
        ft_server_name();
    else if (str[0] == "client_max_body_size" && blockName == "server")
        ft_client_max_body_size();
    else if (str[0] == "upload_path")
        ft_upload_path();
    else if (str[0] == "root")
        ft_root();
    else if (str[0] == "methods")
        ft_methods();
    else if (str[0] == "error_page")
        ft_error_page();
    else if (str[0] == "index")
        ft_index();
    else if (str[0] == "autoindex")
        ft_autoindex();
    else if (str[0] == "}")
    {
        if (blockName == "location")
            blockName = "server";
    }
    else if (str[0] == "return")
        ft_return();
    else if (str[0] == "authInfo")
        ft_authInfo();
    else if (str[0] == "cgi_data")
        ft_cgi_data();
    else
        throw std::runtime_error("erorr in element");
}

void ft_function(Location &obj)
{
    obj.conf.delet_data["methods"] = true;
    obj.conf.delet_data["error_page"] = true;
    obj.conf.delet_data["index"] = true;
    obj.conf.delet_data["cgiExt"] = true;
    obj.conf.delet_data["authInfo"] = true;
}

void Webserv::config_file(std::string nameFielConfig)
{
    count = 0;
    try
    {
        std::ifstream file((nameFielConfig).c_str());
        if (!file)
            throw std::runtime_error("Error opening file");

        std::string sstr;
        bool emptyFile = false;
        while (getline(file, sstr))
        {
            Config cnf;
            emptyFile = true;
            cnf.line = sstr;
            cnf.splite();
            if (cnf.size == 0)
                continue;
            if ((cnf.str.size() == 1 && cnf.str[0] == "server") || (cnf.str.size() == 2 && cnf.str[0] == "server" && cnf.str[1] == "{"))
            {
                count++;
                if (cnf.str.size() == 1)
                {
                    getline(file, sstr);
                    cnf.line = sstr;
                    cnf.splite();
                    if (cnf.str.size() != 1 || cnf.str[0] != "{")
                        throw std::runtime_error("error in opening curly brace in server");
                }
                cnf.blockName = "server";
                while (getline(file, sstr))
                {
                    cnf.line = sstr;
                    cnf.splite();
                    if (cnf.size == 0) //|| str[0][0] == '#'
                        continue;
                    else if (cnf.str[0] == "location")
                    {
                        count++;
                        if (cnf.size != 3 || cnf.blockName == "location" || cnf.str[1][0] != '/' || cnf.str[2] != "{")
                            throw std::runtime_error("location start error");
                        Location obj;
                        obj.conf = cnf;
                        ft_function(obj);
                        obj.location_name = cnf.str[1];
                        while (getline(file, sstr))
                        {
                            obj.conf.line = sstr;
                            obj.conf.splite();
                            if (obj.conf.size == 0)
                                continue;
                            if (obj.conf.str[0] == "}")
                            {
                                if (obj.conf.str.size() > 1)
                                    throw std::runtime_error("error in closing curly brace in location");
                                count--;
                                break;
                            }
                            obj.conf.function();
                            // obj.conf.delet_data = false;
                        }
                        cnf.location.push_back(obj);
                    }
                    else
                    {
                        if (cnf.str[0] == "}")
                        {
                            if (cnf.str.size() > 1)
                                throw std::runtime_error("error in closing curly brace in server");
                            cnf.check_default();
                            this->config.push_back(cnf);
                            count--;
                            break;
                        }
                        cnf.function();
                    }
                }
            }
            else
                throw std::runtime_error("Server start error");
            // cnf.check_default();
            // while (getline(file, sstr))
            // {
            //     cnf.line = sstr;
            //     cnf.splite();
            //     if (cnf.str.size() > 0)
            //         throw std::runtime_error("An error occurred after the server finished");
            // }

        }
        if (emptyFile == false || count != 0)
            throw std::runtime_error("empty config file or {}");
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }
}
