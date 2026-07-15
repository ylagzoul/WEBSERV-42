#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#define MAXPORT 65535
#define MAXIP 256
#define RDWR_SIZE 8192

#include "../server/server.hpp"

// # define LIST_FILE_PATH "/home/ylagzoul/goinfre/"
// # define BODY_FILE_PATH "/home/ylagzoul/goinfre/"

# define LIST_FILE_PATH "/home/mradouan/goinfre/"
# define BODY_FILE_PATH "/home/mradouan/goinfre/"



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
        int                         autoIndex;
        int                         retCode;
        std::string                 retPath;
        std::string                 loginToken;
        std::vector<std::string>    redirectTokens;
        std::vector<std::string>    cgiExt;
        std::vector<std::string>    cgiPath;

        std::vector<Location>       location;
        std::string                 line;
        std::vector<std::string>    words;
        int                         size;
        std::map<std::string, bool> sameLocation;
        std::string                 blockName;
        std::map<std::string, bool> delet_data;

        Config();

        void ft_listen(void);
        void ft_listen_Ip(void);
        void ft_listen_prot(void);
        void ft_listen_prot_Ip(void);
        void ft_root(void);
        void ft_methods();
        void ft_error_page(void);
        void ft_client_max_body_size(void);
        void ft_upload_path();
        void ft_index();
        void ft_autoindex();
        void ft_return();
        void ft_authInfo();
        void ft_cgi_data();

        void splite();
        void check_Directives();
        void check_default();
};

class Location
{
    public:
        Config                      conf;
        std::string                 location_name;
};

std::string ft_check_Path(std::string fullPath);
bool ft_str_char(const char *str, char c);
void checkValid(const std::string& str);

#endif