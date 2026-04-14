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
    // conf.server_name = "server";
    // conf.autoIndex = 0;
    // conf.root = "/home/abouhki/Desktop/webserv_p/";
    Request req;

    req.status = 0;
    req.method = "GET";
    req.path = "response";
    req.httpVersion = "HTTP/1.0"; 
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";
    std::string res = respond(req, conf);
    // std::cout << res;
}
