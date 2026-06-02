#ifndef RECORD_HPP
#define RECORD_HPP

#include "server.hpp"

class Record
{
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
    t_state clientState;
    playloadData data;
    std::string fileName;
    std::string buffer;
    // ~Record(){std::cout << "dist\n\n";}
    std::vector<int> idFdPipe;
};

#endif