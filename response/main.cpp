#include "Response.hpp"
// req.headers["Accept-Language"] = "en-us";
// req.headers["Content-Type"] = "application/json";
// req.headers["Content-Length"] = "11";
// req.bodyFile = "hello world!";
// req.headers["User-Agent"] = "curl/7.16.3";

int main()
{
    Config conf;
    conf.config_file();
    std::cout << "root: " <<  conf.root << std::endl;
    Server server;
    // conf.server_name = "server";
    // conf.autoIndex = 0;
    // conf.root = "/home/abouhki/Desktop/webserv_p/";

    std::string res = respond(server, conf);
    std::cout << res;
}
