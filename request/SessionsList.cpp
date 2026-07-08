#include "SessionsList.hpp"

void    SessionsList::createUnicId()
{
    std::string alphaS = "abcdefghijklmnopqrstuvwxyz";
    std::string alphaU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string digits = "0123456789";

    id = "";
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            int random = std::rand();
            if (j == 0 && i != 0)
                id += '-';
            if (random % 3 == 1)
                id += alphaS[std::rand() % (alphaS.length() - 1)];
            else if (random % 3 == 2)
                id += alphaU[std::rand() % (alphaU.length() - 1)];
            else
                id += digits[std::rand() % (digits.length() - 1)];
        }
    }
}

void SessionsList::generateSession(Request &req)
{
    if (req.cookies.find("session_id") != req.cookies.end())
    {
        id = req.cookies["session_id"];
        if (!id.empty() && sessionLis.find(id) != sessionLis.end())
        {
            req.session = &sessionLis[id];
            req.session->lastLog = std::time(NULL);
            if (req.session->logged == true && req.session->email == sessionLis[id].email)
                req.isLoggedCookie = true;
        }
    }
}

void SessionsList::storeInputs(Request &req, std::string email)
{
        createUnicId();
        Sessions tmpSession;
        tmpSession.email = email;
        tmpSession.lastLog = std::time(NULL);
        tmpSession.logged = true;
        req.isLoggedCookie = true;
        sessionLis[id] = tmpSession;
        req.cookies["session_id"] = id;
        req.session = &sessionLis[id];
}

SessionsList::SessionsList()
{
    std::srand(std::time(NULL));
}


void    SessionsList::checkLimitSessions()
{
    for (std::map<std::string, Sessions>::iterator it = sessionLis.begin(); it != sessionLis.end();)
    {
        time_t now = std::time(NULL);
        if ( now - it->second.lastLog > 30)
            sessionLis.erase(it++);
        else
            it++;
    }
}
bool    SessionsList::checkIfNotNull()
{
    if (!this->sessionLis.empty())
        return (true); 
    else
        return (false);
}