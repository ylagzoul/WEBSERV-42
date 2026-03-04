#include "config_file.hpp"


void Config::splite(Config &conf)
{
    std::string word;
    std::stringstream ss(conf.line);

    conf.str.clear();
    while (ss >> word)
        conf.str.push_back(word);
    conf.size = conf.str.size();
}


void Config::function(Config &conf)
{
    if (conf.str[0] == "listen")
        ft_listen(conf);
    else if (conf.str[0] == "server_name")
        ft_server_name(conf);
    else if (str[0] == "error_page")
        ft_error_page(conf);
}


void Config::config_file(Config& conf)
{
    ser_num = 0;
    try {
        std::ifstream file("config.txt");

        while (getline(file, line))
        {
            splite(conf);
            if ((str[0] == "server" && str.size() == 1) 
                || (str[0] == "server" && str.size() >= 2 && str[1] == "{" && str.size() == 2))
            {
                if (str[1] == "{" )
                    ser_num++;
                while (getline(file, line))                                                                                                                                                    
                {
                    splite(conf);
                    function(conf);
                    if (str[0] == "}")
                    {
                        ser_num--;
                        break ;
                    }
                }
                std::cout << ser_num << std::endl;
                if (ser_num != 0)
                    throw std::runtime_error("erorr {} server");
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


int main()
{
    Config obj;
    obj.config_file(obj);

    std::cout << "listen  = " << obj.get_listen() << std::endl;
    std::cout << "server_name  = " << obj.get_server_name() << std::endl;
}

