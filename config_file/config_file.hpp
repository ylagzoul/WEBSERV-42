#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <iostream>
#include <fstream>
#include <sstream>


class Config {
    private:
        std::string listen;
        std::string server_name;

        int         size;
        std::string *str;
        std::string line;
    public:
        void config_file(Config& conf);
        void splite(Config &conf);
        void function(Config &conf);

        void ft_listen(Config &conf);
        void ft_listen_prot_Ip(Config &conf);
        void ft_listen_prot(Config &conf);
        void ft_listen_Ip(Config &conf);

        void ft_server_name(Config& conf);

        // get && set
        std::string get_listen(void);
        std::string get_server_name(void);

        // class erorr : public std::exception {
        //     public:
        //         const char *what() const throw() {
        //             return ("error\n");
        //         }
        // };
};


bool ft_str_char(const char *str, char c);

#endif