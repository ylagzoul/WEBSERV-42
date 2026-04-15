#ifndef SERVER_HPP
# define SERVER_HPP
#include <iostream>

typedef enum e_state
{
    NEW_RS = 100,
    ACTVE_RS = 101,
    CLOSED_RS = 102,
    GET_RS = 103,
    POST_RS = 104

} t_state;

class Server {
    public:
        int sendSize;
        int sendNum;
        int maxSend;
        bool sendNext;
        bool rspEnd;
        std::string reqEnd;
        int recvSize;
        int recvNum;
        bool recvNext;
};

class Record { 
    public:
        int fd;
        t_state status;
        t_state method;
};

#endif