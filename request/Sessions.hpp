#ifndef SESSIONS_HPP
#define SESSIONS_HPP

#include <ctime>
#include <string>

class Sessions
{
    public:
        std::string email;
        bool        logged;
        time_t      lastLog;
        Sessions();
};

#endif