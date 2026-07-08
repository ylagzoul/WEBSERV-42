#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#define MAXPORT 65535
#define MAXIP 256
#define RDWR_SIZE 8192

// #define LIST_FILE_PATH "/home/ylagzoul/goinfre/"
// #define BODY_FILE_PATH "/home/ylagzoul/goinfre/"

# define LIST_FILE_PATH "/home/mradouan/goinfre/"
# define BODY_FILE_PATH "/home/mradouan/goinfre/"

#include "../server/server.hpp"

// # define LIST_FILE_PATH "/home/abouhki/goinfre/"
// # define BODY_FILE_PATH "/home/abouhki/goinfre/"

class Location;

class Config
{
public:

    std::vector<std::string>    listen_Ip;
    std::vector<int>            listen_Port;
    std::string                 root;
    std::map<std::string, bool> methods;
    std::map<int, std::string>  error_page;
    size_t                      client_max_body_size;
    std::string                 uploadPath;
    std::vector<std::string>    index;
    std::string                 autoindex;
    std::string                 retPath;
    std::vector<std::string>    cgiExt;
    std::vector<std::string>    cgiPath;
    std::string                 loginToken;
    std::vector<std::string>    redirectTokens;
    std::vector<Location>       location;
    
    int                         retCode;
    int                         size;
    int                         ser_num;
    int                         autoIndex;
    std::vector<std::string>    str;
    std::map<std::string, bool> delet_data;
    std::map<std::string, bool> someLocation;
    std::string                 blockName;
    std::string                 line;

    Config();
    
    void ft_listen(void);
    void ft_listen_prot_Ip(void);
    void ft_listen_prot(void);
    void ft_listen_Ip(void);
    void ft_root(void);
    void ft_error_page(void);
    void ft_methods();
    void ft_client_max_body_size(void);
    void ft_index();
    void ft_autoindex();
    void ft_upload_path();
    void ft_return();
    void ft_authInfo();
    void ft_cgi_data();
    
    void splite();
    void function();
    void check_default();

};

class Location
{
public:
    std::string                 location_name;
    std::vector<std::string>    cheke;
    Config                      conf;
};

std::string cleanPath(std::string fullPath);
bool ft_str_char(const char *str, char c);

#endif