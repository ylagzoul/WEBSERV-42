#ifndef SERVER_HPP
# define SERVER_HPP
#include <iostream>


class Servere {
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

#endif