// #include "../server/server.hpp"

// int main()
// {
//     Config conf;
//     conf.config_file();
//     Server server;
//     conf.uploadPath = "upload";
//     Request req;
//     req.status = 0;
//     req.method = "POST";
//     req.path = "/response/testfiles";
//     req.httpVersion = "HTTP/1.0"; 
//     req.headers["Host"] = "localhost";
//     req.headers["Connection"] = "Keep-Alive";

//     std::vector<Record> logs;
//     logs.push_back(Record());

//     std::string res = respond(logs[0], server, conf, req);
//     std::cout  << res << std::endl;
// }
