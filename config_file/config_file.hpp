#ifndef CONFIG_FILE_HPP
# define CONFIG_FILE_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

# define MAXPORT 65535
# define MAXIP 256

# define LIST_FILE_PATH "/home/ylagzoul/goinfre/"
# define BODY_FILE_PATH "/home/ylagzoul/goinfre/"
# define CONF_FILE_PATH "/home/ylagzoul/Desktop//WEBSERV-42/config_file/file_config_file/config.nginx"

// # define LIST_FILE_PATH "/home/mradouan/Desktop/WEBSERV-42/server"
// # define BODY_FILE_PATH "/home/mradouan/goinfre/"
// # define CONF_FILE_PATH "/home/mradouan/Desktop/WEBSERV-42/config_file/file_config_file/moha.nginx"

// # define LIST_FILE_PATH "/home/abouhki/goinfre"
// # define BODY_FILE_PATH "/home/abouhki/goinfre"
// # define CONF_FILE_PATH "/home/abouhki/Desktop/owebserv/config_file/file_config_file/boha.nginx"

class Location;

class Config {
    public:
        std::vector<std::string> listen_Ip;
        std::vector<int> listen_Port;
        std::string server_name;
        std::string root;
        std::map<std::string, bool> methods;
        std::map<int,std::string> error_page;
        std::string blockName;
        size_t client_max_body_size;
        std::string uploadPath;
        std::vector<std::string> index;
        std::string autoindex;
        int retCode;
        std::string retPath;
        std::vector<std::string> cgiExt;
        std::vector<std::string> cgiPath;

        bool emptyFile;

        int                      size;
        int                      ser_num;
        int                      autoIndex;
        size_t                    count;
        std::vector<std::string> str;
        std::string              line;
        std::vector<Location> location;
        void config_file();
        void splite();
        void function();
        Config();

        void ft_listen(void);
        void ft_listen_prot_Ip(void);
        void ft_listen_prot(void);
        void ft_listen_Ip(void);
        void ft_server_name(void);
        void ft_root(void);
        void ft_error_page(void);
        void ft_methods();
        void ft_client_max_body_size(void);
        void ft_index();
        void ft_autoindex();
        void ft_upload_path();
        void ft_return();
        void ft_cgi_data();

        void check_default();

        // get
        std::vector<std::string> get_listen_Ip(void);
        std::vector<std::string> get_listen_Port(void);
        std::string get_server_name(void);
        std::string get_root(void);
        bool get_methods(std::string str);
        std::string get_error_page(int num);
        std::vector<std::string> get_index(void);
        std::string get_autoindex();

};


class Location {
    public:
        std::string location_name;
        std::vector<std::string> cheke;
        Config conf;
};



bool ft_str_char(const char *str, char c);

#endif