#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

// class Location {
//     private:
//         Config conf;

// };


class Config {
    private:
        std::string listen;
        std::string server_name;
        std::string root;
        std::map<int,std::string> error_page;
        // std::string blockName;
        // std::vector<Location> location;

        int                      size;
        int                      ser_num;
        std::vector<std::string> str;
        std::string              line;
    public:
        void config_file(Config& conf);
        void splite(Config &conf);
        void function(Config &conf);

        void ft_listen();
        void ft_listen_prot_Ip();
        void ft_listen_prot();
        void ft_listen_Ip();

        void ft_server_name();
        void ft_root();

        void ft_error_page();

        // get
        std::string get_listen(void);
        std::string get_server_name(void);
        std::string get_root(void);
        std::string get_error_page(int num);

};


bool ft_str_char(const char *str, char c);

#endif