#ifndef RECORD_HPP
# define RECORD_HPP

#include "server.hpp"

class Record { 
    public:
        Record();
        void clean();
        int fd;
        int fileFd;
        int k;
        int number;
        DIR *dir;
        t_state status;
        t_state method;
        playloadData data;
        std::string fileName;
        std::string buffer;
        ~Record();
};

#endif