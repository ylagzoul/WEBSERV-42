#include "../server/server.hpp"

// req.headers["Accept-Language"] = "en-us";
// req.headers["Content-Type"] = "application/json";
// req.headers["Content-Length"] = "11";
// req.bodyFile = "hello world!";
// req.headers["User-Agent"] = "curl/7.16.3";

int main(int ac, char **av)
{
    Config conf;
    conf.config_file();
    Server server;

    Request req;
    (void)ac;
    req.status = 0;
    req.method = "GET";
    req.path = av[1];
    req.httpVersion = "HTTP/1.0"; 
    req.headers["Host"] = "localhost";
    req.headers["Connection"] = "Keep-Alive";

    std::vector<Record> logs;
    logs.push_back(Record());

    std::string res = respond(logs[0], server, conf, req);
    std::cout  << res << std::endl;
}
