#include "../config_file.hpp"


std::string Config::get_listen(void)
{
    return (listen);
}

std::string Config::get_server_name(void)
{
    return (server_name);
}
std::string Config::get_root(void)
{
    return (root);
}

std::string Config::get_error_page(int num)
{
    return (error_page[num]);
}