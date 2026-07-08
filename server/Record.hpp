#ifndef RECORD_HPP
#define RECORD_HPP

class Record
{
public:
    Record();
    void clean();
    int fd;
    int fileFd;
    DIR *dir;
    t_state status;
    t_state method;
    t_state clientState;
    PlayloadData data;
    std::string fileName;
    std::string buffer;
    CgiHandler cgiData;
    std::vector<int> IndexPipe;
    std::vector<int> IndexFdsPipe;

    time_t time;
    std::string response;
    bool sendNext;
};

#endif
