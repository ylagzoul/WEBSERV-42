#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#define MAXPORT 65535
#define MAXIP 256
#define RDWR_SIZE 8192

#define LIST_FILE_PATH "/home/ylagzoul/goinfre/"
#define BODY_FILE_PATH "/home/ylagzoul/goinfre/"

// # define LIST_FILE_PATH "/home/mradouan/Desktop/WEBSERV-42/server"
// # define BODY_FILE_PATH "/home/mradouan/goinfre/"

// # define LIST_FILE_PATH "/home/abouhki/goinfre/"
// # define BODY_FILE_PATH "/home/abouhki/goinfre/"

class Location;

class Config
{
public:
    std::vector<std::string> listen_Ip;
    std::vector<int> listen_Port;
    std::string root;
    std::map<std::string, bool> methods;
    std::map<int, std::string> error_page;
    std::string blockName;
    size_t client_max_body_size;
    std::string uploadPath;
    std::vector<std::string> index;
    std::string autoindex;
    int retCode;
    std::string retPath;
    std::vector<std::string> cgiExt;
    std::vector<std::string> cgiPath;

    std::string loginToken;
    std::vector<std::string> redirectTokens;

    int size;
    int ser_num;
    int autoIndex;
    std::map<std::string, bool> delet_data;
    std::vector<std::string> str;
    std::string line;
    std::vector<Location> location;
    std::map<std::string, bool> someLocation;

    void splite();
    void function();
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
    void check_default();

};

class Location
{
public:
    std::string location_name;
    std::vector<std::string> cheke;
    Config conf;
};

std::string cleanPath(std::string fullPath);
bool ft_str_char(const char *str, char c);

#endif