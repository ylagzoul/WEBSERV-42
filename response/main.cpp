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
    conf.root = "/home/abouhki/Desktop/or_WEBSERV/response/testFolder/";

    Server server;

    Request req;
    req.status = 0;
    req.method = "GET";
    req.path = "file";
    req.httpVersion = "HTTP/1.0"; 
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";

    std::vector<Record> logs;
    logs.push_back(Record());

    std::string res = respond(logs[0], server, conf, req);
    std::cout  << res << std::endl;
}
