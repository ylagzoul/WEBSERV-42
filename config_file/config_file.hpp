#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <iostream>
#include <fstream>
#include <sstream>


class Config {
    private:
        int         size;
        std::string *str;
        // std::string *word;
        std::string line;
        // ll
        std::string listen;
        std::string server_name;
    public:
        void config_file(Config& conf);
        void splite(Config &conf);
        void function(Config &conf);

        void ft_listen(Config &conf);
        void ft_server_name(Config& conf);

        // get && set
        std::string get_listen(void);
        std::string get_server_name(void);

        class erorr : public std::exception {
            public:
                const char *what() const throw() {
                    return ("error\n");
                }
        };
};


bool ft_str_char(const char *str, char c);

#endif