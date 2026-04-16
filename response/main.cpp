#include "../server/server.hpp"

// req.headers["Accept-Language"] = "en-us";
// req.headers["Content-Type"] = "application/json";
// req.headers["Content-Length"] = "11";
// req.bodyFile = "hello world!";
// req.headers["User-Agent"] = "curl/7.16.3";

int main()
{
    Config conf;
    conf.config_file();
    // std::cout << "root: " <<  conf.root << std::endl;
    // std::cout << "location_name: " <<  conf.location[0].location_name << std::endl;
    // std::cout << "autoindex: " <<  conf.location[0].conf.autoindex << std::endl;
    // std::cout << "autoIndex: " <<  conf.location[0].conf.autoIndex << std::endl;
    // std::cout << "methods: " <<  conf.location[0].conf.methods["GET"] << std::endl;
    // std::cout << "methods: " <<  conf.location[0].conf.methods["DELETE"] << std::endl;
    // std::cout << "methods: " <<  conf.location[0].conf.methods["POST"] << std::endl;
    Server server;
    std::vector<Record> rec;
    rec.push_back(Record());
    conf.server_name = "server";
    conf.autoIndex = 0;
    conf.root = "/home/abouhki/Desktop/or_WEBSERV/";

    std::string res = respond(rec[0], server, conf);
    std::cout << res;
    res = respond(rec[0], server, conf);
    std::cout << res;
}
