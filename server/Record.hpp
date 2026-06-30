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
    int number; //tmp
    DIR *dir;
    t_state status;
    t_state method;
    t_state clientState;
    playloadData data;
    std::string fileName;
    std::string buffer;
    CgiHandler cgiData;
    bool isItCgi;
    std::vector<int> IndexPipe;
    std::vector<int> IndexFdsPipe;
    // int cont; //why?
};

#endif
