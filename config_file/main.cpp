#include "config_file.hpp"





int main()
{
    Mserver obj;
    obj.config_file();
    int i = obj.index_mserver["lagzouli"];

    std::cout << "i   =  " << i << std::endl;   

    // std::cout << "index = " << obj.get_index() << std::endl;
    // std::cout << "get_autoindex = " << obj.get_autoindex() << std::endl;
    std::cout << "listen  = " << obj.mserver[i].get_server_name() << std::endl;

    std::cout << "listen  = " << obj.mserver[i].get_listen_Ip()[0] << std::endl;



    // // std::cout << "listen  = " << obj.mserver[i].get_listen_Port()[0] << std::endl;

    // // std::cout << "listen  = " << obj.mserver[i].get_listen_Port()[1] << std::endl;

    // // std::cout << "listen  = " << obj.get_listen_Ip()[2] << std::endl;
    // // std::cout << "listen  = " << obj.get_listen_Port()[2] << std::endl;

    // std::cout << "ft_root  = " << obj.mserver[i].get_root() << std::endl;

    // std::cout << "ft_root  = " << obj.mserver[i].get_error_page(500) << std::endl;
    // std::cout << "ft_root  = " << obj.mserver[i].get_error_page(501) << std::endl;
    // std::cout << "index  = " << obj.mserver[i].get_index()[0] << std::endl;
    // // std::cout << "index  = " << obj.get_index()[1] << std::endl;
    // // std::cout << "index  = " << obj.get_index()[2] << std::endl;
    // // std::cout << "index  = " << obj.get_index()[3] << std::endl;
    // // std::cout << "ft_root  = " << obj.get_index() << std::endl;
    // // std::cout << "server_name  = " << obj.get_server_name() << std::endl;
    // // std::cout << "root  = " << obj.get_root() << std::endl;
    // std::cout << "methods  = " << obj.mserver[i].get_methods("GET") << std::endl;
    // std::cout << "methods  = " << obj.mserver[i].get_methods("POST") << std::endl;
    // std::cout << "location_methods  = " << obj.mserver[i].get_methods("GET") << std::endl;
    // std::cout << "location_methods  = " << obj.mserver[i].get_methods("POST") << std::endl;

    // // std::cout << "error_page  = " << obj.get_error_page(404) << std::endl;
    // // std::cout << "error_page  = " << obj.location[0].conf.get_error_page(404) << std::endl;
    // // std::cout << "error_page  = " << obj.location[1].conf.get_error_page(404) << std::endl;
    // // // std::cout << "error_page  = " << obj.location[2].conf.get_error_page(404) << std::endl;
    // // std::cout << "location_methods  = " << obj.location[0].conf.get_methods_POST("POST") << std::endl;
    // // std::cout << "location_methods  = " << obj.location[0].conf.get_methods("GET") << std::endl;
    // // std::cout << "location_root = " << obj.location[0].conf.get_root() << std::endl;
    // // std::cout << "location_root = " << obj.location[1].conf.get_root() << std::endl;
    // // std::cout << "location_root = " << obj.location[2].conf.get_root() << std::endl;
    // // std::cout << "index = " << obj.location[0].conf.get_index() << std::endl;
    // // std::cout << "get_autoindex = " << obj.location[0].conf.get_autoindex() << std::endl;
    return (1);
}