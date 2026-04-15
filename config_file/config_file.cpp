#include "config_file.hpp"


Config::Config()
{
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


void Config::function()
{
    if (str[0] == "listen" && blockName == "server")
        ft_listen();
    else if (str[0] == "server_name" && blockName == "server")
        ft_server_name();
    else if (str[0] == "client_max_body_size" && blockName == "server")
        ft_client_max_body_size();
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
    else
    {
        std::cout << str[0] << std::endl;
        throw std::runtime_error("name");
    }
}

void Config::config_file()
{
    ser_num = 0;
    try {
        std::ifstream file("file_config_file/config.txt");
        std::string sstr;
        while (getline(file, sstr))
        {
            line = sstr;
            splite();
            if ((str[0] == "server" && str.size() == 1) 
                || (str[0] == "server" && str.size() >= 2 && str[1] == "{" && str.size() == 2))
            {
                blockName = "server";
                while (getline(file, sstr))                                                                                                                                               
                {
                    line = sstr;
                    splite();
                    if (str[0] == "location")
                    {
                        if (size != 3 || blockName == "location" || str[2] != "{")
                            throw std::runtime_error("error loction");
                        Location obj;
                        obj.conf = *this;
                        obj.location_name = str[1];
                        blockName = "location";
                        while(getline(file, sstr) && obj.conf.blockName == "location")
                        {
                            obj.conf.line = sstr;
                            obj.conf.splite();
                            if (obj.conf.str[0] == "}")
                                break;
                            obj.conf.function();
                        }
                        this->location.push_back(obj);
                        blockName = "server";
                    }
                    else
                    {
                        // if (str[0] == "}")
                        //     break ;
                        this->function();
                    }
                }
            }
            else
                throw std::runtime_error("line server");
        }
    }
    catch (const std::exception& e) 
    {
        std::cout << e.what()  << std::endl;
        exit(1);
    }
} 
