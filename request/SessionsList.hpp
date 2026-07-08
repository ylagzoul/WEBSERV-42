#ifndef SESSIONSLIST_HPP
#define SESSIONSLIST_HPP

#include "Sessions.hpp"
#include "Request.hpp"

#include <iostream>
#include <map>

class Request;

class SessionsList
{
    private:
        std::map<std::string, Sessions> sessionLis;
        std::string id;
    public:
        SessionsList(/* args */);
        // ~SessionsList();
        void    createUnicId();
        void    printSesions();
        void    generateSession(Request &req);
        void    checkLimitSessions();
        void    storeInputs(Request &req, std::string email);
        bool    checkIfNotNull();

};


#endif