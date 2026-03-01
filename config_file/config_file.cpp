#include "config_file.hpp"


void Config::splite(Config &conf)
{
    int i = 0;
    std::string word;
    std::stringstream ss(conf.line);

    while (ss >> word)
        i++;

    conf.str = new std::string[i];
    std::stringstream ss1(conf.line);
    i = 0;
    while (ss1 >> word)
        conf.str[i++] = word;
    conf.size = i;
}


void Config::function(Config &conf)
{
    if (conf.str[0] == "listen")
        ft_listen(conf);
    else if (conf.str[0] == "server_name")
        ft_server_name(conf);
}


void Config::config_file(Config& conf)
{
    try {
        std::ifstream file("config.txt");

        while (getline(file, line))
        {
            splite(conf);
            if ((str[0] == "server" && str[1] == "\0") || (str[0] == "server" && str[1] == "{" && str[2] == "\0"))
            {
                while (getline(file, line))                                                                                                                                                    
                {
                    splite(conf);
                    function(conf);
                }
            }
            else
            {
                std::cout << "line server\n";
                throw erorr();
            }
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