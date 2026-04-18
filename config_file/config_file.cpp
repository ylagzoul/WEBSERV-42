#include "../server/server.hpp"


Config::Config()
{
    server_name = "";
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
        std::ifstream file("../config_file/file_config_file/config.nginx");
        std::string sstr;
        while (getline(file, sstr))
        {
            line = sstr;
            splite();
            if ((str[0] == "server" && str.size() == 1) // problem {
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
                    else if (size == 0)
                        continue;
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


// int main()
// {
//     Config obj;
//     obj.config_file();
//     obj.get_server_name();

//     // std::cout << "index = " << obj.get_index() << std::endl;
//     // std::cout << "get_autoindex = " << obj.get_autoindex() << std::endl;
//     std::cout << "listen  = " << obj.get_listen_Ip()[0] << std::endl;
//     std::cout << "listen  = " << obj.get_listen_Port()[0] << std::endl;

//     std::cout << "listen  = " << obj.get_listen_Ip()[1] << std::endl;
//     std::cout << "listen  = " << obj.get_listen_Port()[1] << std::endl;

//     std::cout << "listen  = " << obj.get_listen_Ip()[2] << std::endl;
//     std::cout << "listen  = " << obj.get_listen_Port()[2] << std::endl;

//     std::cout << "ft_root  = " << obj.get_root() << std::endl;

//     std::cout << "ft_root  = " << obj.get_error_page(500) << std::endl;
//     std::cout << "ft_root  = " << obj.get_error_page(501) << std::endl;
//     std::cout << "index  = " << obj.get_index()[0] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[1] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[2] << std::endl;
//     // std::cout << "index  = " << obj.get_index()[3] << std::endl;
//     // std::cout << "ft_root  = " << obj.get_index() << std::endl;
//     // std::cout << "server_name  = " << obj.get_server_name() << std::endl;
//     // std::cout << "root  = " << obj.get_root() << std::endl;
//     std::cout << "methods  = " << obj.get_methods("GET") << std::endl;
//     std::cout << "methods  = " << obj.get_methods("POST") << std::endl;
//     std::cout << "location_methods  = " << obj.get_methods("GET") << std::endl;
//     std::cout << "location_methods  = " << obj.get_methods("POST") << std::endl;
//     // std::cout << "error_page  = " << obj.get_error_page(404) << std::endl;
//     // std::cout << "error_page  = " << obj.location[0].conf.get_error_page(404) << std::endl;
//     // std::cout << "error_page  = " << obj.location[1].conf.get_error_page(404) << std::endl;
//     // // std::cout << "error_page  = " << obj.location[2].conf.get_error_page(404) << std::endl;
//     // std::cout << "location_methods  = " << obj.location[0].conf.get_methods_POST("POST") << std::endl;
//     // std::cout << "location_methods  = " << obj.location[0].conf.get_methods("GET") << std::endl;
//     // std::cout << "location_root = " << obj.location[0].conf.get_root() << std::endl;
//     // std::cout << "location_root = " << obj.location[1].conf.get_root() << std::endl;
//     // std::cout << "location_root = " << obj.location[2].conf.get_root() << std::endl;
//     // std::cout << "index = " << obj.location[0].conf.get_index() << std::endl;
//     // std::cout << "get_autoindex = " << obj.location[0].conf.get_autoindex() << std::endl;
// }

