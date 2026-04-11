#include "../config_file.hpp"


std::vector<std::string> Config::get_listen_Ip(void)
{
    return (listen_Ip);
}

std::vector<std::string> Config::get_listen_Port(void)
{
    return (listen_Port);
}

std::string Config::get_server_name(void)
{
    return (server_name);
}
std::string Config::get_root(void)
{
    return (root);
}

bool Config::get_methods(std::string str)
{
    return (methods[str]);
}

// bool Config::get_methods_POST(std::string str)
// {
//     return (methods[str]);
// }

std::string Config::get_error_page(int num)
{
    return (error_page[num]);
}

std::vector<std::string> Config::get_index(void)
{
    return (index);
}

std::string Config::get_autoindex()
{
    return (autoindex);
}

