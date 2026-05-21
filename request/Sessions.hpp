#ifndef SESSIONS_HPP
#define SESSIONS_HPP

#include <ctime>
#include <string>

// #include "request.hpp"

class Sessions
{
    public:
        std::string userName;
        time_t      lastLog;
        Sessions();
};

#endif