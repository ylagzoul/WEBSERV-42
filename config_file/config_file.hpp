#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <iostream>
#include <fstream>
#include <sstream>


class Config {
    private:
        int         size;
        std::string *str;
        std::string line;
        // ll
        std::string listen;
    public:
        Config& config_file(Config& conf);
        void splite(Config &conf);
        void function(Config &conf);
        void listen(Config &conf);
};




#endif