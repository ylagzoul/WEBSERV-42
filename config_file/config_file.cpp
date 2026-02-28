#include "config_file.hpp"


void Config::splite(Config &conf)
{
    int i = 0;
    std::string word;
    std::stringstream ss(conf.line);

    while (ss >> word)
        i++;

    std::string str[i];
    std::stringstream ss1(conf.line);
    i = 0;
    while (ss1 >> word)
        str[i++] = word;
    conf.size = i;
    conf.str = str;
}

void Config::listen(Config &conf)
{
    if (size != 2)
    {
        std::cout << "error listen\n";
        exit(1);
    }
}


void Config::function(Config &conf)
{
    if (conf.str[0] == "listen")
        listen(conf);
    else if (conf.str[0] == "server_name")
        ;
}


Config& Config::config_file(Config& conf)
{
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
            exit(1);
        }
    }


    return (conf);
} 


int main()
{
    Config obj;
    config_file(obj);
}