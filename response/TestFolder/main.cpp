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
    Server server;
    std::vector<Record> logs;
    logs.push_back(Record());
    logs.push_back(Record());
    conf.root = "/home/abouhki/Desktop/or_WEBSERV/";
    std::string res = respond(logs[0], server, conf);
    std::cout << '[' << res << ']'<< std::endl;
    res = respond(logs[1], server, conf);
    std::cout << '[' << res << ']'<< std::endl;
    res = respond(logs[0], server, conf);
    std::cout << '[' << res << ']'<< std::endl;
    res = respond(logs[1], server, conf);
    std::cout << '[' << res << ']'<< std::endl;
}
